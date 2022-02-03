#ifndef EQUALIZER_H_
#define EQUALIZER_H_

#include "esp_err.h"
#include "audio_element.h"
#include "eq_iir_filter.h"

typedef struct {
    int max_sample;                  /*!< The number of samples per call to process */
    int out_rb_size;                 /*!< Size of ring buffer */
    int task_stack;                  /*!< Size of task stack */
    int task_core;                   /*!< Task running in core... */
    int task_prio;                   /*!< Task priority (based on the FreeRTOS priority) */
    bool stack_in_ext;               /*!< Try to allocate stack in external memory */
    const dfi_iir_filter* iir_filter;
} equalizer2_cfg_t;

#define EQUALIZER2_TASK_STACK (4 * 1024)
#define EQUALIZER2_TASK_CORE (1)
#define EQUALIZER2_TASK_PRIO (5)
#define EQUALIZER2_RINGBUFFER_SIZE (8 * 2048)
#define EQUALIZER2_BUF_SIZE (256)

#define DEFAULT_EQUALIZER2_CONFIG()                                      \
    {                                                                 \
        .max_sample = EQUALIZER2_BUF_SIZE,                              \
        .out_rb_size = EQUALIZER2_RINGBUFFER_SIZE,                       \
        .task_stack = EQUALIZER2_TASK_STACK,                             \
        .task_core = EQUALIZER2_TASK_CORE,                               \
        .task_prio = EQUALIZER2_TASK_PRIO,                               \
        .stack_in_ext = true,                                         \
        .iir_filter = g_equalizer_filters,                              \
    }

audio_element_handle_t equalizer2_init(equalizer2_cfg_t* config);
int equalizer2_process(audio_element_handle_t self, char *in_buffer, int in_len);
esp_err_t equalizer2_open(audio_element_handle_t self);
esp_err_t equalizer2_close(audio_element_handle_t self);
esp_err_t equalizer2_destroy(audio_element_handle_t self);

esp_err_t equalizer2_set_gain(audio_element_handle_t self, unsigned band_idx, float gain_db);
esp_err_t equalizer2_set_enabled(audio_element_handle_t self, bool enabled);

#endif // EQUALIZER_H_