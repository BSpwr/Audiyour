#include "bt_init.h"
#include "profile.h"

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

static const char* TAG = "BT_INIT";

void bt_init(void) {
    esp_err_t err;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_enable(ESP_BT_MODE_BTDM)) != ESP_OK) {
        ESP_LOGE(TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    /* set up bt device name */
    esp_bt_dev_set_device_name(g_device_name.name);

    /* set discoverable and connectable mode, wait to be connected */
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
}


void bt_deinit(void) {
    esp_err_t err;

    if ((err = esp_bluedroid_disable()) != ESP_OK) {
        ESP_LOGE(TAG, "%s esp_bluedroid_disable failed: %s\n",  __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_deinit()) != ESP_OK) {
        ESP_LOGE(TAG, "%s esp_bluedroid_deinit failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_disable()) != ESP_OK) {
        ESP_LOGE(TAG, "%s esp_bt_controller_disable failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_deinit()) != ESP_OK) {
        ESP_LOGE(TAG, "%s esp_bt_controller_deinit failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }
}