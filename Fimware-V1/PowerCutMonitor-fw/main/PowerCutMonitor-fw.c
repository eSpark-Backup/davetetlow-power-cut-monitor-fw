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
    gpio_set_direction(POWER_GPIO, GPIO_MODE_INPUT);
}

void app_main(void)
{
    configure_GPIO();
    int level = gpio_get_level(POWER_GPIO);
    ESP_LOGI(TAG, "GPIO level: %d", level);
    
    if(level == 1){
       esp_deep_sleep_enable_gpio_wakeup(POWER_GPIO, 1); 
       vTaskDelay(pdMS_TO_TICKS(100));
       esp_deep_sleep_start();
    } 
    else {
        ESP_LOGI(TAG, "GPIO is LOW, continuing normal operation...");
    }
}
