#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define POWER_GPIO 5
static const char *TAG = "example";

static void configure_GPIO(void)
{
    // ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    /* Set the GPIO as a push/pull input */
    gpio_reset_pin(POWER_GPIO);
    gpio_set_direction(POWER_GPIO, GPIO_MODE_INPUT);
}

void app_main(void)
{
    configure_GPIO();
    while(1){
    int level = gpio_get_level(POWER_GPIO);
    ESP_LOGI(TAG, "GPIO level: %d", level);
    vTaskDelay(pdMS_TO_TICKS(500));
    if(level == 1){
        ESP_LOGI(TAG, "GPIO is HIGH, continuing normal operation...");
        
    } 
    else {
        ESP_LOGI(TAG, "GPIO is LOW, enter  sleep mode");
        esp_deep_sleep_enable_gpio_wakeup(1 << POWER_GPIO, 1); 
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_light_sleep_start();
        ESP_LOGI(TAG, "Wake up from sleep mode");
    }
}
}
