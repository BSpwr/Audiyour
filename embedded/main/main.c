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
#include "bt_init.h"

#define UART_MONITOR_BAUDRATE (115200)

// extern "C" {
//     void app_main(void);
// }

static const char* TAG = "AUDIYOUR MAIN";

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
    fs_load_profiles(g_profiles, &g_profile_idx, MAX_NUM_PROFILES);
    fs_devicename_load(&g_device_name);

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
