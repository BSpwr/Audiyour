#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "pipeline.h"

extern audiyour_pipeline_a2dp g_audiyour_pipeline;

extern int8_t g_equalizer_gains[10];
extern float g_equalizer_gain_ratios[10];
// {bluetooth_input_gain, line_in_input_gain}

extern bool g_mixer_enable_line_in;
extern bool g_mixer_enable_bluetooth_a2dp_in;
extern int8_t g_source_gains[2];
extern float g_source_gain_ratios[2];

extern int8_t g_output_gain;

void update_g_source_gain_ratios();
void update_g_equalizer_gain_ratios();
float gain_to_ratio(float gain);

#endif // GLOBALS_H_
