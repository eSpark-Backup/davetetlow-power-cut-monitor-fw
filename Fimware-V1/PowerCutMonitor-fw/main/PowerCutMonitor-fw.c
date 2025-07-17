#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

#include "monitor_battery.h"
#include "cloud_handling.h"

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
    configure_GPIO();
    adc_init();

    wakeup_by_power_cutoff_config();

    ESP_ERROR_CHECK(wifi_connect());

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

            send_report_to_server(json_packet);
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
