#ifndef EQ_IIR_FILTER_H_
#define EQ_IIR_FILTER_H_

#define EQ_NUM_BANDS 10
#define IIR_FILTER_NUM_STAGES 3
typedef float dfi_iir_filter[IIR_FILTER_NUM_STAGES][6];
typedef float dfi_iir_filter_buffer[IIR_FILTER_NUM_STAGES][4];

extern const dfi_iir_filter g_equalizer_filters[EQ_NUM_BANDS];

#endif  // EQ_IIR_FILTER_H_