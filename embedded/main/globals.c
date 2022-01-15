#include "globals.h"

audiyour_pipeline_a2dp g_audiyour_pipeline;

int8_t g_equalizer_gains[10]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// {bluetooth_input_gain, line_in_input_gain}
int8_t g_source_gains[2]        = {0x0, 0x0};
int8_t g_output_gain         = 0x0;