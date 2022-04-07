#ifndef GATT_H_
#define GATT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_gatt_common_api.h"
#include "equalizer.h"

#define MAX_EQ_GAIN (10)
#define MIN_EQ_GAIN (-20)

enum
{
    IDX_SVC,
    IDX_CHAR_EQ_GAINS,
    IDX_CHAR_EQ_GAINS_VAL,

    IDX_CHAR_EQ_ENABLE,
    IDX_CHAR_EQ_ENABLE_VAL,
    // IDX_CHAR_CFG_A,

    IDX_CHAR_MIXER_INPUT_GAINS,
    IDX_CHAR_MIXER_INPUT_GAINS_VAL,

    IDX_CHAR_MIXER_ENABLE_JACK_IN,
    IDX_CHAR_MIXER_ENABLE_JACK_IN_VAL,

    IDX_CHAR_MIXER_ENABLE_BLUETOOTH_A2DP_IN,
    IDX_CHAR_MIXER_ENABLE_BLUETOOTH_A2DP_IN_VAL,

    IDX_CHAR_PROFILE_INDEX,
    IDX_CHAR_PROFILE_INDEX_VAL,

    IDX_CHAR_PROFILE_SAVE,
    IDX_CHAR_PROFILE_SAVE_VAL,

    IDX_CHAR_PROFILE_LOAD,
    IDX_CHAR_PROFILE_LOAD_VAL,

    IDX_CHAR_DEVICENAME,
    IDX_CHAR_DEVICENAME_VAL,

    IDX_CHAR_VALIDATE_CONNECTION,
    IDX_CHAR_VALIDATE_CONNECTION_VAL,

    HRS_IDX_NB,
};

audio_element_handle_t equalizer;

typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

#define GATT_TAG     "GATT_SERVER"

void gatts_profile_event_handler(esp_gatts_cb_event_t event,
					esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

void ble_gatts_init(void);

void ble_gatts_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // GATT_H_