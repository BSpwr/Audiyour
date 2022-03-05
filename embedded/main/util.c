#include "util.h"
#include "esp_log.h"
#include "math.h"

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