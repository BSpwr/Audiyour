#ifndef MYCOMP_H_
#define MYCOMP_H_

#include "esp_err.h"
#include "audio_element.h"

#define NUM_BANDS 10

#define IIR_FILTER_NUM_STAGES 5
typedef struct IIR_FILTER {
    float taps[IIR_FILTER_NUM_STAGES][6];
    float buffer[IIR_FILTER_NUM_STAGES][4];
} IIR_FILTER;

audio_element_handle_t mycomp_init();
int mycomp_process(audio_element_handle_t self, char *in_buffer, int in_len);
esp_err_t mycomp_open(audio_element_handle_t self);
esp_err_t mycomp_close(audio_element_handle_t self);
esp_err_t mycomp_destroy(audio_element_handle_t self);


#endif // MYCOMP_H_