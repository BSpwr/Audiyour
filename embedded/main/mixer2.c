#include <string.h>
#include "mixer2.h"
#include "esp_log.h"
#include "util.h"
#include "eq_iir_filter.h"
#include "audio_mem.h"
#include "math.h"

static const char *TAG = "MIXER2";

typedef struct mixer_t {
    size_t max_sample;
    uint8_t **inbuf;
    uint8_t *outbuf;
    float *gain_ratios;
    mixer2_profile profile;
} mixer_t;

audio_element_handle_t mixer2_init(mixer2_cfg_t* config) {
    audio_element_cfg_t cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    cfg.open = mixer2_open;
    cfg.process = mixer2_process;
    cfg.close = mixer2_close;
    cfg.destroy = mixer2_destroy;
    cfg.buffer_len = 0;
    cfg.tag = "mixer";
    cfg.task_stack = config->task_stack;
    cfg.task_prio = config->task_prio;
    cfg.task_core = config->task_core;
    cfg.out_rb_size = config->out_rb_size;
    cfg.stack_in_ext = config->stack_in_ext;
    cfg.multi_in_rb_num = config->profile.num_sources;
    audio_element_handle_t audio_element = audio_element_init(&cfg);

    mixer_t* mixer = audio_calloc(1, sizeof(mixer_t));
    if (mixer == NULL) {
        ESP_LOGE(TAG, "Failed to audio_calloc. (line %d)", __LINE__);
        return NULL;
    }
    audio_element_setdata(audio_element, mixer);

    mixer->max_sample = config->max_sample;
    mixer->profile.num_sources = config->profile.num_sources;

    if (mixer->profile.settings.gains == NULL) {
        mixer->profile.settings.gains = (float*)audio_calloc(mixer->profile.num_sources, sizeof(float));
        if (mixer->profile.settings.gains == NULL) {
            ESP_LOGE(TAG, "Failed to audio_calloc of mixer->profile.gains. (line %d)", __LINE__);
            return NULL;
        }
    }
    if (mixer->profile.settings.enabled == NULL) {
        mixer->profile.settings.enabled = (bool*)audio_calloc(mixer->profile.num_sources, sizeof(bool));
        if (mixer->profile.settings.gains == NULL) {
            ESP_LOGE(TAG, "Failed to audio_calloc of mixer->profile.enabled. (line %d)", __LINE__);
            return NULL;
        }
    }
    mixer->gain_ratios = audio_calloc(mixer->profile.num_sources, sizeof(float));
    mixer2_set_settings(audio_element, config->profile.settings);

    ESP_LOGI(TAG, "init");
    return audio_element;
}

esp_err_t mixer2_open(audio_element_handle_t self) {
    mixer_t *mixer = (mixer_t *)audio_element_getdata(self);

    AUDIO_MEM_SHOW(TAG);

    mixer->inbuf = (uint8_t **)audio_calloc(mixer->profile.num_sources, sizeof(uint8_t *));
    if (mixer->inbuf == NULL) {
        ESP_LOGE(TAG, "Failed to audio_calloc. (line %d)", __LINE__);
        return ESP_ERR_NO_MEM;
    }
    for (unsigned i = 0; i < mixer->profile.num_sources; i++) {
        mixer->inbuf[i] = (uint8_t *)audio_calloc(mixer->max_sample * 2, sizeof(int16_t));
        if (mixer->inbuf[i] == NULL) {
            ESP_LOGE(TAG, "Failed to audio_calloc of mixer->inbuf[%d]. (line %d)", i, __LINE__);
            return ESP_ERR_NO_MEM;
        }
    }
    mixer->outbuf = (uint8_t *)audio_calloc(mixer->max_sample * 2, sizeof(int16_t));
    if (mixer->outbuf == NULL) {
        ESP_LOGE(TAG, "Failed to audio_calloc of mixer->outbuf. (line %d)", __LINE__);
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "open");
    return ESP_OK;
}

int actually_mixer_process(audio_element_handle_t self, unsigned *bytes_filled) {
    mixer_t *mixer = (mixer_t *)audio_element_getdata(self);

    int16_t left_sample, right_sample;
    float work_left, work_right;

    memset(mixer->outbuf, 0, mixer->max_sample * 2 * sizeof(int16_t));
    // https://www.esp32.com/viewtopic.php?f=20&t=20171
    for (int i = 0; i < mixer->max_sample * 2 * sizeof(short); i+=4) {
        work_left = 0;
        work_right = 0;

        for (unsigned source_idx = 0; source_idx < mixer->profile.num_sources; ++source_idx) {

            if (mixer->profile.settings.enabled[source_idx] && i < bytes_filled[source_idx]) {

                memcpy(&right_sample, &mixer->inbuf[source_idx][i], sizeof(int16_t));
                memcpy(&left_sample, &mixer->inbuf[source_idx][i+2], sizeof(int16_t));

                work_right += i16_to_float(right_sample) * mixer->gain_ratios[source_idx];
                work_left += i16_to_float(left_sample) * mixer->gain_ratios[source_idx];
            }
        }

        right_sample = float_to_i16(work_right);
        left_sample = float_to_i16(work_left);

        memcpy(&mixer->outbuf[i], &right_sample, sizeof(int16_t));
        memcpy(&mixer->outbuf[i+2], &left_sample, sizeof(int16_t));
    }

    return mixer->max_sample * 2 * sizeof(short);
}

int mixer2_process(audio_element_handle_t self, char *in_buffer, int in_len) {
    mixer_t *mixer = (mixer_t *)audio_element_getdata(self);

    unsigned channel_bytes = mixer->max_sample * sizeof(int16_t);
    unsigned* bytes_filled = (unsigned*)audio_calloc(mixer->profile.num_sources, sizeof(unsigned));
    int status_end = 0;

    for (unsigned source_idx = 0; source_idx < mixer->profile.num_sources; ++source_idx) {
        memset(mixer->inbuf[source_idx], 0, channel_bytes * 2);

        if (rb_bytes_filled(audio_element_get_multi_input_ringbuf(self, source_idx))) {
            bytes_filled[source_idx] = audio_element_multi_input(self, (char *)mixer->inbuf[source_idx], channel_bytes * 2, source_idx, 50);
        }

        // if (bytes_filled[source_idx] != r_size * 2) {
        //     memset(mixer->inbuf[source_idx] + bytes_filled[source_idx], 0, mixer->max_sample * 2 * sizeof(int16_t) - bytes_filled[source_idx]);
        // }

        ESP_LOGD(TAG, "bytes[ %d ] = %d", source_idx, bytes_filled[source_idx]);
    }

    int ret = actually_mixer_process(self, bytes_filled);

    audio_free(bytes_filled);
    bytes_filled = NULL;

    return audio_element_output(self, (char *)mixer->outbuf, ret);
}

inline esp_err_t mixer2_set_settings(audio_element_handle_t self, mixer2_settings settings) {
    mixer_t *mixer = (mixer_t *)audio_element_getdata(self);

    memcpy(mixer->profile.settings.gains, settings.gains, mixer->profile.num_sources * sizeof(float));
    memcpy(mixer->profile.settings.enabled, settings.enabled, mixer->profile.num_sources * sizeof(bool));

    // convert dB to ratio and store the ratio
    for (unsigned i = 0; i < mixer->profile.num_sources; ++i) {
        mixer->gain_ratios[i] = gain_to_ratio(mixer->profile.settings.gains[i]);
        ESP_LOGI(TAG, "set source ratio[%d] = %f", i, mixer->gain_ratios[i]);
    }

    return ESP_OK;
}

// inline esp_err_t mixer2_set_settings2(audio_element_handle_t self, mixer2_settings_2 settings) {
//     mixer_t *mixer = (mixer_t *)audio_element_getdata(self);

//     assert(mixer->profile.num_sources == 2);

//     memcpy(mixer->profile.settings.gains, settings.gains, mixer->profile.num_sources * sizeof(float));
//     memcpy(mixer->profile.settings.enabled, settings.enabled, mixer->profile.num_sources * sizeof(bool));

//     // convert dB to ratio and store the ratio
//     for (unsigned i = 0; i < mixer->profile.num_sources; ++i) {
//         mixer->gain_ratios[i] = gain_to_ratio(mixer->profile.settings.gains[i]);
//         ESP_LOGI(TAG, "set source ratio[%d] = %f", i, mixer->gain_ratios[i]);
//     }

//     return ESP_OK;
// }

inline esp_err_t mixer2_set_gain(audio_element_handle_t self, unsigned source_idx, float gain_db) {
    mixer_t *mixer = (mixer_t *)audio_element_getdata(self);
    mixer->profile.settings.gains[source_idx] = gain_db;
    // convert dB to ratio and store the ratio
    mixer->gain_ratios[source_idx] = gain_to_ratio(mixer->profile.settings.gains[source_idx]);
    ESP_LOGI(TAG, "set source ratio[%d] = %f", source_idx, mixer->gain_ratios[source_idx]);
    return ESP_OK;
}

esp_err_t mixer2_set_enable(audio_element_handle_t self, unsigned source_idx, bool enabled) {
    mixer_t *mixer = (mixer_t *)audio_element_getdata(self);
    mixer->profile.settings.enabled[source_idx] = enabled;
    ESP_LOGI(TAG, "set enable");
    return ESP_OK;
}

esp_err_t mixer2_close(audio_element_handle_t self) {
    ESP_LOGI(TAG, "close");
    return ESP_OK;
}

esp_err_t mixer2_destroy(audio_element_handle_t self) {
    ESP_LOGI(TAG, "destroy");

    mixer_t *mixer = (mixer_t *)audio_element_getdata(self);

    for (unsigned i = 0; i < mixer->profile.num_sources; i++) {
        audio_free(mixer->inbuf[i]);
    }
    audio_free(mixer->inbuf);
    audio_free(mixer->outbuf);

    audio_free(mixer->profile.settings.enabled);
    mixer->profile.settings.enabled = NULL;
    audio_free(mixer->profile.settings.gains);
    mixer->profile.settings.gains = NULL;
    audio_free(mixer->gain_ratios);
    mixer->gain_ratios = NULL;

    audio_free(mixer);
    mixer = NULL;

    return ESP_OK;
}

void mixer2_set_input_rb(audio_element_handle_t self, ringbuf_handle_t rb, int index) {
    audio_element_set_multi_input_ringbuf(self, rb, index);
}