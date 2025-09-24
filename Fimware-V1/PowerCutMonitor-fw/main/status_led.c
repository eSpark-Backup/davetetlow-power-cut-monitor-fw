#include "status_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static led_state_t current_led_state = LED_STATE_IDLE;

// --- Helper functions ---
static void led_on(gpio_num_t gpio) {
    gpio_set_level(gpio, 1);
}

static void led_off(gpio_num_t gpio) {
    gpio_set_level(gpio, 0);
}

static void led_blink(gpio_num_t gpio, int times, int delay_ms) {
    for (int i = 0; i < times; i++) {
        led_on(gpio);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
        led_off(gpio);
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

// --- LED Task ---
static void led_task(void *pvParameters)
{
    while (1) {
        switch (current_led_state) {
            case LED_STATE_BATTERY_LOW:
                // RED & GREEN blink together
                led_off(STATUS_GREEN);
                led_on(STATUS_RED);
                vTaskDelay(pdMS_TO_TICKS(300));
                led_off(STATUS_RED);
                led_on(STATUS_GREEN);
                vTaskDelay(pdMS_TO_TICKS(300));
                break;

            case LED_STATE_BATTERY_OK:
                // Red blinks once
                
                led_off(STATUS_GREEN);
                led_on(STATUS_RED);
                vTaskDelay(pdMS_TO_TICKS(500));

                
                //current_led_state = LED_STATE_IDLE;
                break;

            case LED_STATE_WIFI_CONNECTED:
                led_on(STATUS_GREEN);
                led_off(STATUS_RED);
                vTaskDelay(pdMS_TO_TICKS(500));
                break;

            case LED_STATE_WIFI_NOT_CONNECTED:
                led_on(STATUS_RED);
                vTaskDelay(pdMS_TO_TICKS(1000));
                led_off(STATUS_RED);
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;

            case LED_STATE_TELEMETRY_FAIL:
                led_blink(STATUS_GREEN, 5, 200);
                vTaskDelay(pdMS_TO_TICKS(500));

                //current_led_state = LED_STATE_IDLE;
                break;

            case LED_STATE_IDLE:
            default:
                led_off(STATUS_RED);
                led_off(STATUS_GREEN);
                vTaskDelay(pdMS_TO_TICKS(500));
                break;
        }
    }
}

// --- Public API ---
void status_led_init(void)
{
    // Configure GPIOs
    gpio_reset_pin(STATUS_RED);
    gpio_reset_pin(STATUS_GREEN);

    gpio_set_direction(STATUS_RED, GPIO_MODE_OUTPUT);
    gpio_set_direction(STATUS_GREEN, GPIO_MODE_OUTPUT);

    // Create LED task
    xTaskCreate(led_task, "led_task", 2048, NULL, 5, NULL);
}

void status_led_set_state(led_state_t state)
{
    current_led_state = state;
}

led_state_t status_led_get_state(void)
{
    return current_led_state;
}
