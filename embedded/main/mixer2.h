#ifndef MIXER_H_
#define MIXER_H_

#include <stdbool.h>
#include "esp_err.h"
#include "audio_element.h"

typedef struct mixer2_settings {
    float *gains;
    bool *enabled;
} mixer2_settings;

// Would use templates, but this is C, not C++
// typedef struct mixer2_settings_2 {
//     float gains[2];
//     bool enabled[2];
// } mixer2_settings_2;

typedef struct mixer2_profile {
    size_t num_sources;
    mixer2_settings settings;
} mixer2_profile;

// typedef struct mixer2_profile_2 {
//     size_t num_sources;
//     mixer2_settings_2 settings;
// } mixer2_profile_2;

typedef struct {
    int max_sample;                  /*!< The number of samples per call to process */
    int out_rb_size;                 /*!< Size of ring buffer */
    int task_stack;                  /*!< Size of task stack */
    int task_core;                   /*!< Task running in core... */
    int task_prio;                   /*!< Task priority (based on the FreeRTOS priority) */
    bool stack_in_ext;               /*!< Try to allocate stack in external memory */
    mixer2_profile profile;
} mixer2_cfg_t;

#define MIXER2_TASK_STACK (8 * 1024)
#define MIXER2_TASK_CORE (0)
#define MIXER2_TASK_PRIO (5)
#define MIXER2_RINGBUFFER_SIZE (8 * 1024)
#define MIXER2_BUF_SIZE (256)

#define MIXER2_NUM_SOURCES (2)
static float default_gains2[MIXER2_NUM_SOURCES] = {0.0f, 0.0f};
static bool default_enabled2[MIXER2_NUM_SOURCES] = {true, true};


#define DEFAULT_MIXER2_CONFIG()                                      \
    {                                                                 \
        .max_sample = MIXER_BUF_SIZE,                                    \
        .out_rb_size = MIXER_RINGBUFFER_SIZE,                       \
        .task_stack = MIXER_TASK_STACK,                             \
        .task_core = MIXER_TASK_CORE,                               \
        .task_prio = MIXER_TASK_PRIO,                               \
        .stack_in_ext = true,                                         \
        .profile = {\
            .num_sources = MIXER2_NUM_SOURCES,\
            .settings = {\
                .gains = default_gains2,\
                .enabled = default_enabled2,\
            },\
        },\
    }\

audio_element_handle_t mixer2_init(mixer2_cfg_t* config);
int mixer2_process(audio_element_handle_t self, char *in_buffer, int in_len);
esp_err_t mixer2_open(audio_element_handle_t self);
esp_err_t mixer2_close(audio_element_handle_t self);
esp_err_t mixer2_destroy(audio_element_handle_t self);

esp_err_t mixer2_set_gain(audio_element_handle_t self, unsigned source_idx, float gain_db);
esp_err_t mixer2_set_settings(audio_element_handle_t self, mixer2_settings profile);
// esp_err_t mixer2_set_settings2(audio_element_handle_t self, mixer2_settings_2 profile);
esp_err_t mixer2_set_enable(audio_element_handle_t self, unsigned source_idx, bool enabled);

void mixer2_set_input_rb(audio_element_handle_t self, ringbuf_handle_t rb, int index);

#endif // MIXER_H_