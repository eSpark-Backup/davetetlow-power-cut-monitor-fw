#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "monitor_battery.h"
#include "cloud_handling.h"
#include "wifi_provisioning.h"

static const char *TAG = "Main";

#define POWER_GPIO 5

static void configure_GPIO(void)
{
    // ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    /* Set the GPIO as a push/pull input */
    gpio_reset_pin(POWER_GPIO);
    gpio_set_direction(POWER_GPIO, GPIO_MODE_INPUT);
}

void wakeup_by_power_cutoff_config()
{
    esp_sleep_enable_gpio_wakeup();
    gpio_wakeup_enable(POWER_GPIO, GPIO_INTR_HIGH_LEVEL);
    esp_rom_gpio_pad_select_gpio(POWER_GPIO);
    gpio_pullup_dis(POWER_GPIO);
    gpio_pulldown_en(POWER_GPIO);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    configure_GPIO();
    adc_init();

    wakeup_by_power_cutoff_config();

    // ESP_ERROR_CHECK(init_ble());

    // nvs_flash_init(); // 1 - Initialize NVS flash using

    // esp_nimble_hci_and_controller_init();

    nimble_port_init(); // 3 - Initialize the host stack

    ble_svc_gap_device_name_set("BLE-Server-Test");
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
    // ESP_ERROR_CHECK(wifi_connect());

    float volts;
    int ad_value = 0;
    while (1)
    {
        volts = batteryMonitoring(&ad_value);
        ESP_LOGI(TAG, "Volts: %.2f and Battery Percentage: %.0f%%", volts, ((volts - 1.8) / (3.0 - 1.8)) * 100);

        int level = gpio_get_level(POWER_GPIO);

        ESP_LOGI(TAG, "GPIO level: %d", level);

        vTaskDelay(pdMS_TO_TICKS(500));

        char *json_packet;

        if (level == 1 || volts < 2.04)
        {
            ESP_LOGI(TAG, "GPIO is %d", level);

            json_packet = create_json_data(level, volts, ad_value);
            if (wifi_connected)
            {
                send_report_to_server(json_packet);
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        else
        {
            ESP_LOGI(TAG, "GPIO is LOW, and battery percentage is %.0f%% entering sleep mode", ((volts - 1.8) / (3.0 - 1.8)) * 100);
            // esp_deep_sleep_enable_gpio_wakeup(1 << POWER_GPIO, 1);
            // esp_light_sleep_start();
            esp_sleep_enable_timer_wakeup(WAKE_INTERVAL_SEC);
            vTaskDelay(pdMS_TO_TICKS(500));
            esp_deep_sleep_start();
            ESP_LOGI(TAG, "Wake up from sleep mode");
        }
    }
}
