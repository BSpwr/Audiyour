#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "driver/i2s.h"

#include "gatt.h"


// --
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"
#include "esp_peripherals.h"

#include "nvs_flash.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "i2s_stream.h"
#include "input_key_service.h"
#include "filter_resample.h"
#include "periph_touch.h"
#include "board.h"
#include "a2dp_stream.h"
#include "driver/uart.h"

#include "raw_stream.h"
#include "equalizer.h"
#include "es8388.h"

#include "pipeline.h"
#include "profile.h"

#define UART_MONITOR_BAUDRATE (115200)

#define AUDIYOUR_MAIN_TAG "AUDIYOUR MAIN"


void bt_init(void) {
    esp_err_t err;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(AUDIYOUR_MAIN_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_enable(ESP_BT_MODE_BTDM)) != ESP_OK) {
        ESP_LOGE(AUDIYOUR_MAIN_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(AUDIYOUR_MAIN_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(AUDIYOUR_MAIN_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    /* set up bt device name */
    esp_bt_dev_set_device_name(BT_DEVICE_NAME);

    /* set discoverable and connectable mode, wait to be connected */
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
}


void bt_deinit(void) {
    esp_err_t err;

    if ((err = esp_bluedroid_disable()) != ESP_OK) {
        ESP_LOGE(AUDIYOUR_MAIN_TAG, "%s esp_bluedroid_disable failed: %s\n",  __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bluedroid_deinit()) != ESP_OK) {
        ESP_LOGE(AUDIYOUR_MAIN_TAG, "%s esp_bluedroid_deinit failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_disable()) != ESP_OK) {
        ESP_LOGE(AUDIYOUR_MAIN_TAG, "%s esp_bt_controller_disable failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

    if ((err = esp_bt_controller_deinit()) != ESP_OK) {
        ESP_LOGE(AUDIYOUR_MAIN_TAG, "%s esp_bt_controller_deinit failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }
}


// --------------------------------------------------------------

// extern "C" {
//     void app_main(void);
// }

const char* TAG = "LITTLE_FS";

void app_main(void)
{
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    uart_config_t uart_config = {
        .baud_rate = UART_MONITOR_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);

    fs_init();
    // fs_profiles_init(MAX_NUM_PROFILES);
    fs_get_profiles(g_profiles, &g_current_profile, MAX_NUM_PROFILES);

    bt_init();
    audiyour_pipeline_a2dp_init(&g_audiyour_pipeline);
    apply_active_profile();
    audiyour_pipeline_a2dp_run(&g_audiyour_pipeline);
    ble_gatts_init();

    profile_writeback_task_create();

    while(1) {
        vTaskDelay(1000);
    }

    profile_writeback_task_delete();

    ble_gatts_deinit();
    audiyour_pipeline_a2dp_deinit(&g_audiyour_pipeline);
    bt_deinit();

    fs_deinit();

}
