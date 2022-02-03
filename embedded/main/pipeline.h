#ifndef PIPELINE_H_
#define PIPELINE_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "audio_element.h"
#include "audio_pipeline.h"
#include "board.h"

// ------ A2DP TASK freeRTOS
#define A2DP_STREAM_TASK_STACK      (2 * 1024)
#define A2DP_STREAM_TASK_CORE       (0)
#define A2DP_STREAM_TASK_PRIO       (22)
#define A2DP_STREAM_TASK_IN_EXT     (true)
#define A2DP_STREAM_QUEUE_SIZE      (4096)
// ------

// ------ EQUALIZER TASK freeRTOS
#define EQUALIZER_TASK_STACK        (4 * 1024)
#define EQUALIZER_TASK_CORE         (1)
#define EQUALIZER_TASK_PRIO         (5)
#define EQUALIZER_RINGBUFFER_SIZE   (8 * 2048)
// ------

// ------ MIXER TASK freeRTOS
#define MIXER_TASK_STACK (8 * 1024)
#define MIXER_TASK_CORE (0)
#define MIXER_TASK_PRIO (5)
#define MIXER_RINGBUFFER_SIZE (8 * 2048)
#define MIXER_BUF_SIZE (256)
// ------ MIXER TASK freeRTOS

typedef struct audiyour_pipeline_a2dp {
    audio_board_handle_t board_handle;
    audio_pipeline_handle_t pipeline;
    audio_pipeline_handle_t pipeline_bt_read;
    audio_pipeline_handle_t pipeline_jack_read;
    audio_element_handle_t bt_stream_reader;
    audio_element_handle_t i2s_stream_writer;
    audio_element_handle_t i2s_stream_reader;
    audio_element_handle_t bt_stream_raw;
    audio_element_handle_t jack_stream_raw;
    audio_element_handle_t mixer;
    audio_element_handle_t equalizer;
    ringbuf_handle_t rb_jack_stream_raw;
    ringbuf_handle_t rb_bt_stream_raw;
    periph_service_handle_t input_ser;
    esp_periph_set_handle_t periph_set;
    esp_periph_handle_t bt_periph;
    audio_event_iface_handle_t evt;
    TaskHandle_t event_listener_task;
    audio_element_handle_t mycomp;
} audiyour_pipeline_a2dp;

// void update_equalizer_gains(audiyour_pipeline_a2dp* audiyour_pipeline, int8_t equalizer_gains[10]);

void update_equalizer_gains(audiyour_pipeline_a2dp* audiyour_pipeline, float equalizer_gains[10]);

void audiyour_pipeline_a2dp_init(audiyour_pipeline_a2dp* audiyour_pipeline);

void audiyour_pipeline_a2dp_deinit(audiyour_pipeline_a2dp* audiyour_pipeline);

#ifdef __cplusplus
}
#endif


#endif // PIPELINE_H_