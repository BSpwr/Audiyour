#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "pipeline.h"

extern audiyour_pipeline_a2dp g_audiyour_pipeline;

extern int8_t g_equalizer_gains[10];
// {bluetooth_input_gain, line_in_input_gain}
extern int8_t g_source_gains[2];
extern int8_t g_output_gain;

#endif // GLOBALS_H_
