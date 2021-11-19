#ifndef BLE_GATT_H_
#define BLE_GATT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Attributes State Machine */
enum
{
    IDX_SVC,
    IDX_CHAR_EQ_GAINS,
    IDX_CHAR_EQ_GAINS_VAL,
    // IDX_CHAR_CFG_A,

    IDX_CHAR_INPUT_GAINS,
    IDX_CHAR_INPUT_GAINS_VAL,

    IDX_CHAR_OUTPUT_GAIN,
    IDX_CHAR_OUTPUT_GAIN_VAL,

    HRS_IDX_NB,
};



#endif // BLE_GATT_H_