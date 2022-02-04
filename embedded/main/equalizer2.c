#include "equalizer2.h"
#include "esp_log.h"
#include "globals.h"
#include "eq_iir_filter.h"
#include "audio_mem.h"
#include "math.h"

static const char *TAG = "EQUALIZER";

typedef struct equalizer_t {
    const dfi_iir_filter* iir_filter;
    dfi_iir_filter_buffer iir_buffer[EQ_NUM_BANDS * 2];
    float equalizer_gain_ratios[EQ_NUM_BANDS];
    bool enabled;
} equalizer_t;

static inline float directform_I_process(const dfi_iir_filter filter, dfi_iir_filter_buffer buffer, float sample) {
    float cascaded_sample = sample;

    for (uint16_t stage_num = 0; stage_num < IIR_FILTER_NUM_STAGES; stage_num++) {
        // [X1 X2 Y1 Y2]
        float* regs = buffer[stage_num];

        // [B0 B1 B2 1 A1 A2]
        const float* taps = filter[stage_num];

        // calculate the feed forward using the B terms:
        // (X0 * B0) + (X1 * B1) + (X2 * B2)
        float feed_forward =
            cascaded_sample * taps[0] + regs[0] * taps[1] + regs[1] * taps[2];
        // calculate the feedback using the A terms: (Y1 * -A1) + (Y2 * -A2)
        float feedback = regs[2] * -taps[4] + regs[3] * -taps[5];
        // output is feed forward terms + feedback terms (A0 is always 1, so the
        // output is not getting scaled)
        float output = feed_forward + feedback;

        regs[1] = regs[0];          // X2 = X1
        regs[0] = cascaded_sample;  // X1 = X0

        regs[3] = regs[2];  // Y2 = Y1
        regs[2] = output;   // Y1 = output

        // feed output of filter into other filter
        cascaded_sample = output;
    }

    return cascaded_sample;
}

audio_element_handle_t equalizer2_init(equalizer2_cfg_t* config) {
    audio_element_cfg_t cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    cfg.open = equalizer2_open;
    cfg.process = equalizer2_process;
    cfg.close = equalizer2_close;
    cfg.destroy = equalizer2_destroy;
    cfg.tag = "equalizer";
    cfg.task_stack = config->task_stack;
    cfg.task_prio = config->task_prio;
    cfg.task_core = config->task_core;
    cfg.out_rb_size = config->out_rb_size;
    cfg.stack_in_ext = config->stack_in_ext;

    audio_element_handle_t audio_element = audio_element_init(&cfg);

    equalizer_t* eq_data = audio_calloc(1, sizeof(equalizer_t));
    AUDIO_MEM_CHECK(TAG, eq_data, return NULL);
    audio_element_setdata(audio_element, eq_data);

    eq_data->iir_filter = config->iir_filter;

    for (unsigned i = 0; i < EQ_NUM_BANDS; ++i) {
        eq_data->equalizer_gain_ratios[i] = 1.0f;
    }
    eq_data->enabled = true;

    return audio_element;
}

esp_err_t equalizer2_open(audio_element_handle_t self) {
    ESP_LOGI(TAG, "open");
    return ESP_OK;
}

int equalizer2_process(audio_element_handle_t self, char *in_buffer, int in_len) {
    equalizer_t *eq_data = (equalizer_t *)audio_element_getdata(self);

    int rsize = audio_element_input(self, in_buffer, in_len);

    if (in_len != rsize || (rsize % 4) != 0) {
            ESP_LOGW(TAG, "unexpected rsize: %d, in_len: %d", rsize, in_len);
        }

    if (eq_data->enabled) {
        int16_t left_sample, right_sample;
        float left_sample_f, right_sample_f;

        float output_left_sample_f, output_right_sample_f;

        char * left_sample_p = (char *) &left_sample;
        char * right_sample_p = (char *) &right_sample;

        for (int i =0; i<rsize; i+=4) {
            right_sample_p[0] = in_buffer[i];
            right_sample_p[1] = in_buffer[i+1];

            left_sample_p[0] = in_buffer[i+2];
            left_sample_p[1] = in_buffer[i+3];

            left_sample_f = i16_to_float(left_sample);
            right_sample_f = i16_to_float(right_sample);

            output_left_sample_f = 0;
            output_right_sample_f = 0;

            for (unsigned j = 0; j < 10; ++j) {
                output_left_sample_f += directform_I_process(eq_data->iir_filter[j], eq_data->iir_buffer[j * 2], left_sample_f) * eq_data->equalizer_gain_ratios[j];
                output_right_sample_f += directform_I_process(eq_data->iir_filter[j], eq_data->iir_buffer[j * 2 + 1], right_sample_f) * eq_data->equalizer_gain_ratios[j];
            }

            left_sample = float_to_i16(output_left_sample_f);
            right_sample = float_to_i16(output_right_sample_f);

            in_buffer[i] = right_sample_p[0];
            in_buffer[i+1] = right_sample_p[1];

            in_buffer[i+2] = left_sample_p[0];
            in_buffer[i+3] = left_sample_p[1];
        }
    }

    rsize = audio_element_output(self, in_buffer, rsize);

    return rsize;
}

inline esp_err_t equalizer2_set_gain(audio_element_handle_t self, unsigned band_idx, float gain_db) {
    equalizer_t *eq_data = (equalizer_t *)audio_element_getdata(self);
    eq_data->equalizer_gain_ratios[band_idx] = gain_to_ratio(gain_db);
    ESP_LOGI(TAG, "set band ratio[%d] = %f", band_idx, eq_data->equalizer_gain_ratios[band_idx]);
    return ESP_OK;
}

esp_err_t equalizer2_set_enable(audio_element_handle_t self, bool enabled) {
    equalizer_t *eq_data = (equalizer_t *)audio_element_getdata(self);
    eq_data->enabled = enabled;
    ESP_LOGI(TAG, "set enable");
    return ESP_OK;
}

esp_err_t equalizer2_close(audio_element_handle_t self) {
    ESP_LOGI(TAG, "close");
    return ESP_OK;
}

esp_err_t equalizer2_destroy(audio_element_handle_t self) {
    ESP_LOGI(TAG, "destroy");

    equalizer_t *equalizer = (equalizer_t *)audio_element_getdata(self);
    audio_free(equalizer);
    equalizer = NULL;

    return ESP_OK;
}