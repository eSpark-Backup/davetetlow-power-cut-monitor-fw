#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "monitor_battery.h"
#include "cloud_handling.h"
#include "nvs_store_retrieve.h"
#include "wifi_provisioning.h"

static char *TAG = "Main";

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
    bool ble_mode = false;

    ESP_ERROR_CHECK(nvs_init());

    char *ssid = NULL;
    char *password = NULL;

    if (nvs_read_string("wifi_creds", "ssid", &ssid) == ESP_OK)
    {
        if (nvs_read_string("wifi_creds", "pass", &password) == ESP_OK)
        {
            if (ssid && password)
            {
                if (wifi_connect(ssid, password) == ESP_OK)
                {
                    ESP_LOGE("WIFI", "Wifi Connected****************");
                }
                else
                {
                    ESP_LOGW("WIFI", "Wifi Not Connected****************");
                }
            }
        }
    }
    else
    {
        ESP_LOGI(TAG, "Wifi Credentials Not Found==================================");

        nimble_port_init();

        static uint8_t base_mac[6];
        esp_err_t ret = esp_base_mac_addr_get(base_mac);
        if (ret == ESP_OK)
        {
            ESP_LOGI(TAG, "Base MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
                     base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read base MAC address");
        }
        char mac_address[24];
        snprintf(mac_address, sizeof(mac_address), "ESP_%02X:%02X:%02X:%02X:%02X:%02X", base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);
        ESP_LOGE("MAC", "%s", mac_address);

        ble_svc_gap_device_name_set(mac_address);

        ble_svc_gap_init();
        ble_svc_gatt_init();
        ble_gatts_count_cfg(gatt_svcs);
        ble_gatts_add_svcs(gatt_svcs);

        ble_hs_cfg.sync_cb = ble_app_on_sync;
        nimble_port_freertos_init(host_task);
        ble_mode = true;
        return;
    }

    while (1)
    {
        if (!wifi_connected)
        {
            if (!ble_mode)
            {
                ESP_LOGI(TAG, "Wifi Not Connected BLE Mode is Turning On==================================");
                nimble_port_init();
                ESP_LOGE("BLE", "INITIALIZED");

                static uint8_t base_mac[6];
                esp_err_t ret = esp_base_mac_addr_get(base_mac);
                if (ret == ESP_OK)
                {
                    ESP_LOGI(TAG, "Base MAC Address: %02X:%02X:%02X:%02X:%02X:%02X",
                             base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);
                }
                else
                {
                    ESP_LOGE(TAG, "Failed to read base MAC address");
                }
                char mac_address[24];
                snprintf(mac_address, sizeof(mac_address), "ESP_%02X:%02X:%02X:%02X:%02X:%02X", base_mac[0], base_mac[1], base_mac[2], base_mac[3], base_mac[4], base_mac[5]);
                ESP_LOGE("MAC", "%s", mac_address);

                ble_svc_gap_device_name_set(mac_address);
                ble_svc_gap_init();
                ble_svc_gatt_init();
                ble_gatts_count_cfg(gatt_svcs);
                ble_gatts_add_svcs(gatt_svcs);
                ESP_LOGE("BLE", "INITIALIZED");
                ble_hs_cfg.sync_cb = ble_app_on_sync;
                nimble_port_freertos_init(host_task);
                ble_mode = true;
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }
        else
        {
            ESP_LOGI(TAG, "Wifi Connected==================================");
            configure_GPIO();
            adc_init();
            wakeup_by_power_cutoff_config();
            float volts = batteryMonitoring();
            ESP_LOGI(TAG, "Volts: %.2f and Battery Percentage: %.0f%%", volts, ((volts - 1.8) / (3.0 - 1.8)) * 100);

            int level = gpio_get_level(POWER_GPIO);
            ESP_LOGI(TAG, "GPIO level: %d", level);

            if (level == 1 || volts < 2.04)
            {
                ESP_LOGI(TAG, "GPIO is %d", level);

                char *json_packet = create_json_data_1(level, volts);
                while(send_report_to_server(json_packet) != ESP_OK){};
                ESP_LOGI(TAG, "GPIO is %d, and battery percentage is %.0f%% entering sleep mode", level, ((volts - 1.8) / (3.0 - 1.8)) * 100);
            }
            // esp_deep_sleep_enable_gpio_wakeup(1 << POWER_GPIO, 1);
            esp_sleep_enable_timer_wakeup(WAKE_INTERVAL_SEC);
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP_LOGI(TAG, "Going to sleep!, will wakeup after %d seconds", WAKE_INTERVAL_SEC/(1000*1000));
            esp_deep_sleep_start();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
