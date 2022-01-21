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
    // IDX_CHAR_CFG_A,

    IDX_CHAR_INPUT_GAINS,
    IDX_CHAR_INPUT_GAINS_VAL,

    IDX_CHAR_OUTPUT_GAIN,
    IDX_CHAR_OUTPUT_GAIN_VAL,

    HRS_IDX_NB,
};

audio_element_handle_t equalizer;

typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

#define GATT_TAG     "GATT_SERVER"
#define BT_DEVICE_NAME      "Audiyour"

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