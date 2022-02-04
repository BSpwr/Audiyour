#include "globals.h"
#include "esp_log.h"
#include "math.h"

audiyour_pipeline_a2dp g_audiyour_pipeline;

float g_equalizer_gains[10]    = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
bool g_equalizer_enable = true;
// float g_equalizer_gain_ratios[10] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

bool g_mixer_enable_line_in = true;
bool g_mixer_enable_bluetooth_a2dp_in = true;
// g_source_gains[0] is 3.5mm jack, g_source_gains[1] is bluetooth a2dp
int8_t g_source_gains[2]        = {0x0, 0x0};
float g_source_gain_ratios[2]  = {1.0f, 1.0f};

int8_t g_output_gain         = 0x0;

inline void update_g_source_gain_ratios() {
    g_source_gain_ratios[0] = gain_to_ratio((float)g_source_gains[0]);
    g_source_gain_ratios[1] = gain_to_ratio((float)g_source_gains[1]);
    const char* tag = "SOURCE_GAIN_RATIO_UPDATE";
    ESP_LOGI(tag, "source_gain_ratio[0]: %f", g_source_gain_ratios[0]);
    ESP_LOGI(tag, "source_gain_ratio[1]: %f", g_source_gain_ratios[1]);
    ESP_LOGI(tag, "source_gains[0]: %d", g_source_gains[0]);
    ESP_LOGI(tag, "source_gains[1]: %d", g_source_gains[1]);
}

// inline void update_g_equalizer_gain_ratios() {
//     for (unsigned i = 0; i < 10; i++) {
//         g_equalizer_gain_ratios[i] = gain_to_ratio((float)g_equalizer_gains[i]);
//     }
// }

inline int16_t float_to_i16(float input) {
    if (input > INT16_MAX) return INT16_MAX;
    if (input < INT16_MIN) return INT16_MIN;
    return (int16_t)input;
}

inline float i16_to_float(int16_t input) {
    return (float)input;
}

inline float gain_to_ratio(float gain) {
    return powf(10, gain / 20);
}