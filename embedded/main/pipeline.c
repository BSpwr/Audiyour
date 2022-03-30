#include "nvs_flash.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "i2s_stream.h"
#include "input_key_service.h"
#include "filter_resample.h"
#include "periph_touch.h"
#include "board.h"
#include "a2dp_stream.h"
#include "driver/uart.h"

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_peripherals.h"

#include "mixer.h"
#include "raw_stream.h"
#include "equalizer.h"
#include "es8388.h"

#include "util.h"
#include "pipeline.h"
#include "equalizer2.h"

#include <time.h>

clock_t time_before;

static const char *TAG = "PIPELINE";

audiyour_pipeline_a2dp g_audiyour_pipeline;

void pipeline_update_equalizer_gains(audiyour_pipeline_a2dp* audiyour_pipeline, float equalizer_gains[10]) {
    clock_t new_time = clock();
    ESP_LOGI(TAG, "Update_equalizer_gains");
    if (audiyour_pipeline && audiyour_pipeline->equalizer && (float)(new_time - time_before)/CLOCKS_PER_SEC >= 0.25) {
        for (int i = 0; i < 10; i++) {
            equalizer2_set_gain(audiyour_pipeline->equalizer, i, equalizer_gains[i]);
        }
        time_before = clock();
    }
}

void pipeline_update_equalizer_enable(audiyour_pipeline_a2dp* audiyour_pipeline, bool enabled) {
    ESP_LOGI(TAG, "Update_equalizer_enable");
    if (audiyour_pipeline && audiyour_pipeline->equalizer) {
        equalizer2_set_enable(audiyour_pipeline->equalizer, enabled);
    }
}

void pipeline_update_mixer_gain(audiyour_pipeline_a2dp* audiyour_pipeline, unsigned source_idx, float gain_db) {
    ESP_LOGI(TAG, "Update_mixer_gain");
    if (audiyour_pipeline && audiyour_pipeline->mixer) {
        ESP_LOGI(TAG, "mixer value: %f", gain_db);
        mixer_set_gain(audiyour_pipeline->mixer, source_idx, gain_db);
    }
}

void pipeline_update_mixer_enable(audiyour_pipeline_a2dp* audiyour_pipeline, unsigned source_idx, bool enabled) {
    ESP_LOGI(TAG, "Update_mixer_enable");
    if (audiyour_pipeline && audiyour_pipeline->mixer) {
        mixer_set_enable(audiyour_pipeline->mixer, source_idx, enabled);
    }
}

static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    ESP_LOGI(TAG, "INSIDE input_key_service_cb");
    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
        ESP_LOGI(TAG, "[*] input key id is %d", (int)evt->data);
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_PLAY:
                ESP_LOGI(TAG, "[*] [Play] play");
                periph_bt_play(g_audiyour_pipeline.bt_periph);
                break;
            case INPUT_KEY_USER_ID_SET:
                ESP_LOGI(TAG, "[*] [Set] pause");
                periph_bt_pause(g_audiyour_pipeline.bt_periph);
                break;
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0))
            case INPUT_KEY_USER_ID_VOLUP:
                ESP_LOGI(TAG, "[*] [long Vol+] Vol+");
                periph_bt_volume_up(g_audiyour_pipeline.bt_periph);
                break;
            case INPUT_KEY_USER_ID_VOLDOWN:
                ESP_LOGI(TAG, "[*] [long Vol-] Vol-");
                periph_bt_volume_down(g_audiyour_pipeline.bt_periph);
                break;
#endif
        }
    } else if (evt->type == INPUT_KEY_SERVICE_ACTION_PRESS) {
        ESP_LOGI(TAG, "[*] input key id is %d", (int)evt->data);
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_VOLUP:
                ESP_LOGI(TAG, "[*] [long Vol+] next");
                periph_bt_avrc_next(g_audiyour_pipeline.bt_periph);
                break;
            case INPUT_KEY_USER_ID_VOLDOWN:
                ESP_LOGI(TAG, "[*] [long Vol-] Previous");
                periph_bt_avrc_prev(g_audiyour_pipeline.bt_periph);
                break;
        }

    }
    return ESP_OK;
}

void pipeline_event_listener_task() {
    while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(g_audiyour_pipeline.evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[*] Event interface error : %d", ret);
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) g_audiyour_pipeline.bt_stream_reader
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(g_audiyour_pipeline.bt_stream_reader, &music_info);

            ESP_LOGI(TAG, "[*] Receive music info from Bluetooth, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);

            audio_element_setinfo(g_audiyour_pipeline.i2s_stream_writer, &music_info);
#if (CONFIG_ESP_LYRATD_MSC_V2_1_BOARD || CONFIG_ESP_LYRATD_MSC_V2_2_BOARD)
#else
            i2s_stream_set_clk(g_audiyour_pipeline.i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
#endif
            continue;
        }

        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) g_audiyour_pipeline.i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[*] Stop event received");
            break;
        }
    }
}

void audiyour_pipeline_a2dp_init(audiyour_pipeline_a2dp* audiyour_pipeline) {
    ESP_LOGI(TAG, "[01] Start codec chip");
    // initialize audio board dac and audio_hal
    audiyour_pipeline->board_handle = audio_board_init();
    audio_hal_ctrl_codec(audiyour_pipeline->board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START); // instead of AUDIO_HAL_CODEC_LINE_IN
    es8388_write_reg(ES8388_ADCCONTROL2, ADC_INPUT_LINPUT2_RINPUT2);
    es8388_write_reg(ES8388_ADCCONTROL1, 0x00);
    es8388_write_reg(ES8388_DACCONTROL24, 0b100001);
    es8388_write_reg(ES8388_DACCONTROL25, 0b100001);

    ESP_LOGI(TAG, "[02] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    audiyour_pipeline->pipeline = audio_pipeline_init(&pipeline_cfg); // pipeline for mixing BT + jack

    audiyour_pipeline->pipeline_bt_read = audio_pipeline_init(&pipeline_cfg);
    audiyour_pipeline->pipeline_jack_read = audio_pipeline_init(&pipeline_cfg);

    ESP_LOGI(TAG, "[03] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_reader_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_reader_cfg.type = AUDIO_STREAM_READER;
    audiyour_pipeline->i2s_stream_reader = i2s_stream_init(&i2s_reader_cfg);
    audio_pipeline_register(audiyour_pipeline->pipeline_jack_read, audiyour_pipeline->i2s_stream_reader, "jack");

    ESP_LOGI(TAG, "[04] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_writer_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_writer_cfg.type = AUDIO_STREAM_WRITER;
    audiyour_pipeline->i2s_stream_writer = i2s_stream_init(&i2s_writer_cfg);

    ESP_LOGI(TAG, "[05] Create A2DP Bluetooth stream");
    a2dp_stream_config_t a2dp_config = {
        .type = AUDIO_STREAM_READER,
        .user_callback = {0},
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0))
        .audio_hal = audiyour_pipeline->board_handle->audio_hal,
#endif
    };
    audiyour_pipeline->bt_stream_reader = a2dp_stream_init(&a2dp_config);
    audio_pipeline_register(audiyour_pipeline->pipeline_bt_read, audiyour_pipeline->bt_stream_reader, "bt");

    ESP_LOGI(TAG, "[06] Setup raw streams for use in mixer");
    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_WRITER;

    audiyour_pipeline->bt_stream_raw = raw_stream_init(&raw_cfg);
    audio_pipeline_register(audiyour_pipeline->pipeline_bt_read, audiyour_pipeline->bt_stream_raw, "bt_raw");
    const char *link_tag_bt_pipeline[4] = {"bt", "bt_raw"};
    audio_pipeline_link(audiyour_pipeline->pipeline_bt_read, &link_tag_bt_pipeline[0], 2);

    audiyour_pipeline->jack_stream_raw = raw_stream_init(&raw_cfg);
    audio_pipeline_register(audiyour_pipeline->pipeline_jack_read, audiyour_pipeline->jack_stream_raw, "jack_raw");
    const char *link_tag_jack_pipeline[4] = {"jack", "jack_raw"};
    audio_pipeline_link(audiyour_pipeline->pipeline_jack_read, &link_tag_jack_pipeline[0], 2);

    ESP_LOGI(TAG, "[06] Create mixer");
    mixer_cfg_t mixer_cfg = DEFAULT_MIXER_CONFIG();
    audiyour_pipeline->mixer = mixer_init(&mixer_cfg);

    ESP_LOGI(TAG, "[07] Create equalizer");
    equalizer2_cfg_t eq2_cfg = DEFAULT_EQUALIZER2_CONFIG();
    audiyour_pipeline->equalizer = equalizer2_init(&eq2_cfg);

    ESP_LOGI(TAG, "[08] Connect raw streams to mixer");
    audiyour_pipeline->rb_jack_stream_raw = audio_element_get_input_ringbuf(audiyour_pipeline->jack_stream_raw);
    mixer_set_input_rb(audiyour_pipeline->mixer, audiyour_pipeline->rb_jack_stream_raw, 0);

    audiyour_pipeline->rb_bt_stream_raw = audio_element_get_input_ringbuf(audiyour_pipeline->bt_stream_raw);
    mixer_set_input_rb(audiyour_pipeline->mixer, audiyour_pipeline->rb_bt_stream_raw, 1);

    ESP_LOGI(TAG, "[09] Register all elements to audio pipeline");
    audio_pipeline_register(audiyour_pipeline->pipeline, audiyour_pipeline->mixer, "mixer");
    audio_pipeline_register(audiyour_pipeline->pipeline, audiyour_pipeline->equalizer, "equalizer");
    audio_pipeline_register(audiyour_pipeline->pipeline, audiyour_pipeline->i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[10] Link it together mixer-->equalizer-->i2s_stream_writer-->[codec_chip]");
    const char *link_tag[3] = {"mixer", "equalizer", "i2s"};
    audio_pipeline_link(audiyour_pipeline->pipeline, &link_tag[0], 3);

    ESP_LOGI(TAG, "[11] Initialize peripherals");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    audiyour_pipeline->periph_set = esp_periph_set_init(&periph_cfg);
    audio_board_key_init(audiyour_pipeline->periph_set);

    ESP_LOGI(TAG, "[12] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = audiyour_pipeline->periph_set;
    audiyour_pipeline->input_ser = input_key_service_create(&input_cfg);
    input_key_service_add_key(audiyour_pipeline->input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(audiyour_pipeline->input_ser, input_key_service_cb, NULL);

    ESP_LOGI(TAG, "[13] Create Bluetooth peripheral");
    audiyour_pipeline->bt_periph = bt_create_periph();

    ESP_LOGI(TAG, "[14] Start all peripherals");
    esp_periph_start(audiyour_pipeline->periph_set, audiyour_pipeline->bt_periph);

    ESP_LOGI(TAG, "[15] Set up event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audiyour_pipeline->evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[16] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(audiyour_pipeline->pipeline_jack_read, audiyour_pipeline->evt);
    audio_pipeline_set_listener(audiyour_pipeline->pipeline_bt_read, audiyour_pipeline->evt);
    audio_pipeline_set_listener(audiyour_pipeline->pipeline, audiyour_pipeline->evt);
    // audio_event_iface_set_listener(esp_periph_set_get_event_iface(audiyour_pipeline->periph_set), audiyour_pipeline->evt);

    time_before = clock();
}

void audiyour_pipeline_a2dp_run(audiyour_pipeline_a2dp* audiyour_pipeline) {
    ESP_LOGI(TAG, "[17] Start audio_pipeline");
    audio_pipeline_run(audiyour_pipeline->pipeline_jack_read);
    audio_pipeline_run(audiyour_pipeline->pipeline_bt_read);
    audio_pipeline_run(audiyour_pipeline->pipeline);

    ESP_LOGI(TAG, "[18] Listen for all pipeline events");

    /* Create the task, storing the handle. */
    BaseType_t ret = xTaskCreate(
                    (void (*)(void *))pipeline_event_listener_task,       /* Function that implements the task. */
                    "audiyour_pipeline_event_listener",          /* Text name for the task. */
                    4096,      /* Stack size in words, not bytes. */
                    NULL,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,/* Priority at which the task is created. */
                    &audiyour_pipeline->event_listener_task);      /* Used to pass out the created task's handle. */

    if (ret == pdPASS) {
        ESP_LOGI(TAG, "Pipeline event handler task started successfully");
        /* The task was created.  Use the task's handle to delete the task. */
    }
}

void audiyour_pipeline_a2dp_deinit(audiyour_pipeline_a2dp* audiyour_pipeline) {
    vTaskDelete(audiyour_pipeline->event_listener_task);

    audio_pipeline_stop(audiyour_pipeline->pipeline);
    audio_pipeline_wait_for_stop(audiyour_pipeline->pipeline);
    audio_pipeline_terminate(audiyour_pipeline->pipeline);

    audio_pipeline_stop(audiyour_pipeline->pipeline_bt_read);
    audio_pipeline_wait_for_stop(audiyour_pipeline->pipeline_bt_read);
    audio_pipeline_terminate(audiyour_pipeline->pipeline_bt_read);

    audio_pipeline_stop(audiyour_pipeline->pipeline_jack_read);
    audio_pipeline_wait_for_stop(audiyour_pipeline->pipeline_jack_read);
    audio_pipeline_terminate(audiyour_pipeline->pipeline_jack_read);

    a2dp_destroy();

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(audiyour_pipeline->pipeline);
    audio_pipeline_remove_listener(audiyour_pipeline->pipeline_bt_read);
    audio_pipeline_remove_listener(audiyour_pipeline->pipeline_jack_read);

    /* Stop all periph before removing the listener */
    esp_periph_set_stop_all(audiyour_pipeline->periph_set);
    // audio_event_iface_remove_listener(esp_periph_set_get_event_iface(audiyour_pipeline->periph_set), audiyour_pipeline->evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(audiyour_pipeline->evt);

    /* Release all resources */
    audio_pipeline_unregister(audiyour_pipeline->pipeline_jack_read, audiyour_pipeline->i2s_stream_reader);
    audio_pipeline_unregister(audiyour_pipeline->pipeline_bt_read, audiyour_pipeline->bt_stream_reader);
    audio_pipeline_unregister(audiyour_pipeline->pipeline_bt_read, audiyour_pipeline->bt_stream_raw);
    audio_pipeline_unregister(audiyour_pipeline->pipeline_jack_read, audiyour_pipeline->jack_stream_raw);
    audio_pipeline_unregister(audiyour_pipeline->pipeline, audiyour_pipeline->mixer);
    // audio_pipeline_unregister(audiyour_pipeline->pipeline, audiyour_pipeline->equalizer);
    audio_pipeline_unregister(audiyour_pipeline->pipeline, audiyour_pipeline->i2s_stream_writer);

    /* Deinit all resources */
    audio_element_deinit(audiyour_pipeline->i2s_stream_reader);
    audio_element_deinit(audiyour_pipeline->bt_stream_reader);
    audio_element_deinit(audiyour_pipeline->bt_stream_raw);
    audio_element_deinit(audiyour_pipeline->jack_stream_raw);
    audio_element_deinit(audiyour_pipeline->mixer);
    // audio_element_deinit(audiyour_pipeline->equalizer);
    audio_element_deinit(audiyour_pipeline->i2s_stream_writer);

    /* Deinit periph service and periph set */
    periph_service_destroy(audiyour_pipeline->input_ser);
    esp_periph_set_destroy(audiyour_pipeline->periph_set);

    /* Deinit audio board */
    audio_board_deinit(audiyour_pipeline->board_handle);
}
