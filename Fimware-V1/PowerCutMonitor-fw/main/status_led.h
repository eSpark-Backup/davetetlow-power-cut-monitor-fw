#ifndef STATUS_LED_H
#define STATUS_LED_H

#include "driver/gpio.h"

// Define LED GPIOs 
#define STATUS_RED    GPIO_NUM_10
#define STATUS_GREEN  GPIO_NUM_6

// LED states
typedef enum {
    LED_STATE_IDLE = 0,
    LED_STATE_BATTERY_LOW,
    LED_STATE_BATTERY_OK,
    LED_STATE_WIFI_CONNECTED,
    LED_STATE_WIFI_NOT_CONNECTED,
    LED_STATE_TELEMETRY_FAIL
} led_state_t;

// Initialize GPIOs and create LED task
void status_led_init(void);

// Update LED state
void status_led_set_state(led_state_t state);

// Get current LED state
led_state_t status_led_get_state(void);

#endif // STATUS_LED_H
