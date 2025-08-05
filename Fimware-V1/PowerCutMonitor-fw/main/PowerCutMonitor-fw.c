#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "monitor_battery.h"
#include "cloud_handling.h"
#include "nvs_store_retrieve.h"
#include "wifi_provisioning.h"

static char *TAG = "Main";

#define POWER_GPIO GPIO_NUM_5

// Print the wakeup reason for debugging
static void print_wakeup_reason(void) {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            ESP_LOGI(TAG, "Wakeup caused by EXT0 (GPIO)");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGI(TAG, "Wakeup caused by timer");
            break;
        case ESP_SLEEP_WAKEUP_GPIO:
            ESP_LOGI(TAG, "Wakeup caused by GPIO (esp_deep_sleep_enable_gpio_wakeup)");
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            ESP_LOGI(TAG, "Wakeup not from deep sleep");
            break;
        default:
            ESP_LOGI(TAG, "Wakeup cause unknown: %d", wakeup_reason);
            break;
    }
}


// static void configure_GPIO(void)
// {
//     // ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
//     /* Set the GPIO as a push/pull input */
//     gpio_reset_pin(POWER_GPIO);
//     gpio_set_direction(POWER_GPIO, GPIO_MODE_INPUT);
// }

void wakeup_by_power_cutoff_config()
{

    print_wakeup_reason();

    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << POWER_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,    // Keep pin HIGH when button not pressed
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    if (!esp_sleep_is_valid_wakeup_gpio(POWER_GPIO)) {
        ESP_LOGE(TAG, "GPIO %d is not a valid RTC GPIO for deep sleep wakeup", POWER_GPIO);
        return;
    }
    else{
        ESP_LOGI(TAG, "GPIO %d is a valid RTC GPIO for deep sleep wakeup", POWER_GPIO);
    }

    esp_err_t err = esp_deep_sleep_enable_gpio_wakeup((1ULL << POWER_GPIO), ESP_GPIO_WAKEUP_GPIO_LOW); // Wake on LOW
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable EXT0 wakeup: %s", esp_err_to_name(err));
        return;
    }
    else{
        ESP_LOGI(TAG, "EXT0 wakeup enabled on GPIO %d", POWER_GPIO);
    }

    
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
            // configure_GPIO();
            adc_init();
            wakeup_by_power_cutoff_config();
            float volts = batteryMonitoring();
            ESP_LOGI(TAG, "Volts: %.2f and Battery Percentage: %.0f%%", volts, ((volts - 1.8) / (3.0 - 1.8)) * 100);

            int level = gpio_get_level(POWER_GPIO);
            ESP_LOGI(TAG, "GPIO level: %d", level);

            if (level == 0 || volts < 2.04)
            {
                ESP_LOGI(TAG, "GPIO is %d", level);

                char *json_packet = create_json_data_1(level, volts);
                while(send_report_to_server(json_packet) != ESP_OK){};
                ESP_LOGI(TAG, "GPIO is %d, and battery percentage is %.0f%% entering sleep mode", level, ((volts - 1.8) / (3.0 - 1.8)) * 100);
            }
            // esp_deep_sleep_enable_gpio_wakeup(1 << POWER_GPIO, 1);
            esp_sleep_enable_timer_wakeup(WAKE_INTERVAL_SEC);
            vTaskDelay(pdMS_TO_TICKS(100));
            ESP_LOGI(TAG, "Going to sleep!, will wakeup after %d minutes", WAKE_INTERVAL_SEC/(1000*1000));
            esp_deep_sleep_start();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
