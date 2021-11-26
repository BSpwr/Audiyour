#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "driver/i2s.h"

#include "gatt.h"


// --
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_peripherals.h"

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

#include "mixer.h"
#include "raw_stream.h"
#include "equalizer.h"
#include "es8388.h"

#define UART_MONITOR_BAUDRATE (115200)

// ------ A2DP TASK freeRTOS
#define A2DP_STREAM_TASK_STACK      (2 * 1024)
#define A2DP_STREAM_TASK_CORE       (0)
#define A2DP_STREAM_TASK_PRIO       (22)
#define A2DP_STREAM_TASK_IN_EXT     (true)
#define A2DP_STREAM_QUEUE_SIZE      (1024)
// ------

// ------ EQUALIZER TASK freeRTOS
#define EQUALIZER_TASK_STACK        (4 * 1024)
#define EQUALIZER_TASK_CORE         (1)
#define EQUALIZER_TASK_PRIO         (5)
#define EQUALIZER_RINGBUFFER_SIZE   (8 * 1024)
// ------

// ------ MIXER TASK freeRTOS
#define MIXER_TASK_STACK (8 * 1024)
#define MIXER_TASK_CORE (0)
#define MIXER_TASK_PRIO (5)
#define MIXER_RINGBUFFER_SIZE (8 * 1024)
#define MIXER_BUF_SIZE (256)
// ------ MIXER TASK freeRTOS

static const char *TAG = "BLUETOOTH_EXAMPLE";
static esp_periph_handle_t bt_periph = NULL;

static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
    if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
        ESP_LOGI(TAG, "[ * ] input key id is %d", (int)evt->data);
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_PLAY:
                ESP_LOGI(TAG, "[ * ] [Play] play");
                periph_bt_play(bt_periph);
                break;
            case INPUT_KEY_USER_ID_SET:
                ESP_LOGI(TAG, "[ * ] [Set] pause");
                periph_bt_pause(bt_periph);
                break;
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0))
            case INPUT_KEY_USER_ID_VOLUP:
                ESP_LOGI(TAG, "[ * ] [long Vol+] Vol+");
                periph_bt_volume_up(bt_periph);
                break;
            case INPUT_KEY_USER_ID_VOLDOWN:
                ESP_LOGI(TAG, "[ * ] [long Vol-] Vol-");
                periph_bt_volume_down(bt_periph);
                break;
#endif
        }
    } else if (evt->type == INPUT_KEY_SERVICE_ACTION_PRESS) {
        ESP_LOGI(TAG, "[ * ] input key id is %d", (int)evt->data);
        switch ((int)evt->data) {
            case INPUT_KEY_USER_ID_VOLUP:
                ESP_LOGI(TAG, "[ * ] [long Vol+] next");
                periph_bt_avrc_next(bt_periph);
                break;
            case INPUT_KEY_USER_ID_VOLDOWN:
                ESP_LOGI(TAG, "[ * ] [long Vol-] Previous");
                periph_bt_avrc_prev(bt_periph);
                break;
        }

    }
    return ESP_OK;
}
// --

// audio_element_handle_t equalizer;



// --------------------------------------------------------------

extern "C" {
    void app_main(void);
}
 
void app_main(void)
{
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    uart_config_t uart_config = {
        .baud_rate = UART_MONITOR_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(BT_BLE_COEX_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_enable(ESP_BT_MODE_BTDM)) != ESP_OK) {
        ESP_LOGE(BT_BLE_COEX_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(BT_BLE_COEX_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(BT_BLE_COEX_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    /* set up bt device name */
    esp_bt_dev_set_device_name(BT_DEVICE_NAME);

    /* set discoverable and connectable mode, wait to be connected */
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    // --
    audio_pipeline_handle_t pipeline;
    audio_element_handle_t bt_stream_reader, i2s_stream_writer;

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    // audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_LINE_IN, AUDIO_HAL_CTRL_START);

    // audio_hal_codec_config_t audio_hal_codec_cfg =  AUDIO_CODEC_DEFAULT_CONFIG();
    // audio_hal_codec_cfg.adc_input = AUDIO_HAL_ADC_INPUT_LINE2;
    // audio_hal_codec_cfg.i2s_iface.samples = AUDIO_HAL_44K_SAMPLES; // here put your sampling mode
    // audio_hal_handle_t hal = audio_hal_init(&audio_hal_codec_cfg, &AUDIO_CODEC_ES8388_DEFAULT_HANDLE);
    // board_handle->audio_hal = hal;
    // audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_LINE_IN, AUDIO_HAL_CTRL_START);
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START); // instead of AUDIO_HAL_CODEC_LINE_IN
    es8388_write_reg(ES8388_ADCCONTROL2, ADC_INPUT_LINPUT2_RINPUT2);
    es8388_write_reg(ES8388_ADCCONTROL1, 0x00);


    ESP_LOGI(TAG, "[ 3 ] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);

    audio_pipeline_handle_t pipeline_bt_read = audio_pipeline_init(&pipeline_cfg);
    audio_pipeline_handle_t pipeline_jack_read = audio_pipeline_init(&pipeline_cfg);

    ESP_LOGI(TAG, "[3.2] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_reader_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_reader_cfg.type = AUDIO_STREAM_READER;
    audio_element_handle_t i2s_stream_reader = i2s_stream_init(&i2s_reader_cfg);
    audio_pipeline_register(pipeline_jack_read, i2s_stream_reader, "jack");

    ESP_LOGI(TAG, "[4] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_writer_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_writer_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_writer_cfg);

    equalizer_cfg_t eq_cfg = DEFAULT_EQUALIZER_CONFIG();

    int set_gain[20];
    for (unsigned int i = 0; i < 10; i++) {
        set_gain[i * 2] = (int)((int8_t)equalizer_gains[i]);
        set_gain[i * 2 + 1] = (int)((int8_t)equalizer_gains[i]);
    }
    eq_cfg.set_gain =
        set_gain; // The size of gain array should be the multiplication of NUMBER_BAND and number channels of audio stream data. The minimum of gain is -13 dB.
    equalizer = equalizer_init(&eq_cfg);

    ESP_LOGI(TAG, "[4.1] Get Bluetooth stream");
    a2dp_stream_config_t a2dp_config = {
        .type = AUDIO_STREAM_READER,
        .user_callback = {0},
#if (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0))
        .audio_hal = board_handle->audio_hal,
#endif
    };
    bt_stream_reader = a2dp_stream_init(&a2dp_config);
    audio_pipeline_register(pipeline_bt_read, bt_stream_reader, "bt");

    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_WRITER;
    audio_element_handle_t bt_stream_raw = raw_stream_init(&raw_cfg);
    audio_pipeline_register(pipeline_bt_read, bt_stream_raw, "bt_raw");

    const char *link_tag_bt_pipeline[4] = {"bt", "bt_raw"};
    audio_pipeline_link(pipeline_bt_read, &link_tag_bt_pipeline[0], 2);

    audio_element_handle_t jack_stream_raw = raw_stream_init(&raw_cfg);
    audio_pipeline_register(pipeline_jack_read, jack_stream_raw, "jack_raw");

    const char *link_tag_jack_pipeline[4] = {"jack", "jack_raw"};
    audio_pipeline_link(pipeline_jack_read, &link_tag_jack_pipeline[0], 2);

    ESP_LOGI(TAG, "[4.2] Register mixer");
#define INDEX_BASE_STREAM 0
#define INDEX_NEWCOME_STREAM 1
#define SAMPLERATE 44100
#define NUM_INPUT_CHANNEL 2
#define TRANSMITTIME 0
#define MUSIC_GAIN_DB 0
#define PLAY_STATUS ESP_DOWNMIX_OUTPUT_TYPE_TWO_CHANNEL
#define NUMBER_SOURCE_FILE 2
    downmix_cfg_t mixer_cfg = DEFAULT_MIXER_CONFIG();
    mixer_cfg.downmix_info.source_num = 2;
    audio_element_handle_t mixer = mixer_init(&mixer_cfg);

    esp_downmix_input_info_t source_information[NUMBER_SOURCE_FILE] = {0};
    esp_downmix_input_info_t source_info_base = {
        .samplerate = SAMPLERATE,
        .channel = NUM_INPUT_CHANNEL,
        .bits_num = 16,
        .gain = {0, 0},
        .transit_time = TRANSMITTIME,
    };
    source_information[0] = source_info_base;

    esp_downmix_input_info_t source_info_newcome = {
        .samplerate = SAMPLERATE,
        .channel = NUM_INPUT_CHANNEL,
        .bits_num = 16,
        .gain = {0, 0},
        .transit_time = TRANSMITTIME,
    };
    source_information[1] = source_info_newcome;
    source_info_init(mixer, source_information);

    ringbuf_handle_t rb_jack_stream_raw = audio_element_get_input_ringbuf(jack_stream_raw);
    mixer_set_input_rb(mixer, rb_jack_stream_raw, 1);

    ringbuf_handle_t rb_bt_stream_raw = audio_element_get_input_ringbuf(bt_stream_raw);
    mixer_set_input_rb(mixer, rb_bt_stream_raw, 0);

    ESP_LOGI(TAG, "[4.2] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, mixer, "mixer");
    audio_pipeline_register(pipeline, equalizer, "equalizer");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[4.3] Link it together [Bluetooth]-->bt_stream_reader-->i2s_stream_writer-->[codec_chip]");
    const char *link_tag[3] = {"mixer", "equalizer", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);

    ESP_LOGI(TAG, "[ 5 ] Initialize peripherals");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);
    audio_board_key_init(set);

    ESP_LOGI(TAG, "[ 5.1 ] Create and start input key service");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    input_key_service_cfg_t input_cfg = INPUT_KEY_SERVICE_DEFAULT_CONFIG();
    input_cfg.handle = set;
    periph_service_handle_t input_ser = input_key_service_create(&input_cfg);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, input_key_service_cb, NULL);

    ESP_LOGI(TAG, "[5.2] Create Bluetooth peripheral");
    bt_periph = bt_create_periph();

    ESP_LOGI(TAG, "[5.3] Start all peripherals");
    esp_periph_start(set, bt_periph);

    ESP_LOGI(TAG, "[ 6 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[6.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline_jack_read, evt);
    audio_pipeline_set_listener(pipeline_bt_read, evt);
    audio_pipeline_set_listener(pipeline, evt);
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);
    mixer_set_output_type(mixer, PLAY_STATUS);
    mixer_set_input_rb_timeout(mixer, 50, INDEX_BASE_STREAM);
    mixer_set_input_rb_timeout(mixer, 50, INDEX_NEWCOME_STREAM);
    mixer_set_work_mode(mixer, ESP_DOWNMIX_WORK_MODE_SWITCH_ON);

    ESP_LOGI(TAG, "[ 7 ] Start audio_pipeline");
    audio_pipeline_run(pipeline_jack_read);
    audio_pipeline_run(pipeline_bt_read);
    audio_pipeline_run(pipeline);
    // --

    //gatt server init
    ble_gatts_init();

    ESP_LOGI(TAG, "[ 8 ] Listen for all pipeline events");
    while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) bt_stream_reader
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(bt_stream_reader, &music_info);

            ESP_LOGI(TAG, "[ * ] Receive music info from Bluetooth, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);

            audio_element_setinfo(i2s_stream_writer, &music_info);
#if (CONFIG_ESP_LYRATD_MSC_V2_1_BOARD || CONFIG_ESP_LYRATD_MSC_V2_2_BOARD)
#else
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
#endif
            continue;
        }

        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
        }
    }
}
