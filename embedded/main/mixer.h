#ifndef MIXER_H_
#define MIXER_H_

#include <stdbool.h>
#include "esp_err.h"
#include "audio_element.h"

typedef struct mixer_settings {
    float *gains;
    bool *enabled;
} mixer_settings;

// Would use templates, but this is C, not C++
// typedef struct mixer_settings_2 {
//     float gains[2];
//     bool enabled[2];
// } mixer_settings_2;

typedef struct mixer_profile {
    size_t num_sources;
    mixer_settings settings;
} mixer_profile;

// typedef struct mixer_profile_2 {
//     size_t num_sources;
//     mixer_settings_2 settings;
// } mixer_profile_2;

typedef struct {
    int max_sample;                  /*!< The number of samples per call to process */
    int out_rb_size;                 /*!< Size of ring buffer */
    int task_stack;                  /*!< Size of task stack */
    int task_core;                   /*!< Task running in core... */
    int task_prio;                   /*!< Task priority (based on the FreeRTOS priority) */
    bool stack_in_ext;               /*!< Try to allocate stack in external memory */
    mixer_profile profile;
} mixer_cfg_t;

#define mixer_TASK_STACK (8 * 1024)
#define mixer_TASK_CORE (0)
#define mixer_TASK_PRIO (5)
#define mixer_RINGBUFFER_SIZE (8 * 1024)
#define mixer_BUF_SIZE (256)

#define mixer_NUM_SOURCES (2)
static float default_gains2[mixer_NUM_SOURCES] = {0.0f, 0.0f};
static bool default_enabled2[mixer_NUM_SOURCES] = {true, true};


#define DEFAULT_MIXER_CONFIG()                                      \
    {                                                                 \
        .max_sample = MIXER_BUF_SIZE,                                    \
        .out_rb_size = MIXER_RINGBUFFER_SIZE,                       \
        .task_stack = MIXER_TASK_STACK,                             \
        .task_core = MIXER_TASK_CORE,                               \
        .task_prio = MIXER_TASK_PRIO,                               \
        .stack_in_ext = true,                                         \
        .profile = {\
            .num_sources = mixer_NUM_SOURCES,\
            .settings = {\
                .gains = default_gains2,\
                .enabled = default_enabled2,\
            },\
        },\
    }\

audio_element_handle_t mixer_init(mixer_cfg_t* config);
int mixer_process(audio_element_handle_t self, char *in_buffer, int in_len);
esp_err_t mixer_open(audio_element_handle_t self);
esp_err_t mixer_close(audio_element_handle_t self);
esp_err_t mixer_destroy(audio_element_handle_t self);

esp_err_t mixer_set_gain(audio_element_handle_t self, unsigned source_idx, float gain_db);
esp_err_t mixer_set_settings(audio_element_handle_t self, mixer_settings profile);
esp_err_t mixer_set_enable(audio_element_handle_t self, unsigned source_idx, bool enabled);

void mixer_set_input_rb(audio_element_handle_t self, ringbuf_handle_t rb, int index);

#endif // MIXER_H_