#ifndef MONITOR_BATTERY_H
#define MONITOR_BATTERY_H

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"


#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_4       // GPIO4 (ADC1_CH4)
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_6       // ADC_ATTEN_DB_11 // Attenuation for higher voltage range
#define EXAMPLE_ADC_BIT_WIDTH ADC_WIDTH_BIT_12 // 12-bit resolution
#define NUM_SAMPLES 150
#define WAKE_INTERVAL_SEC 5*1000*1000 // wake up timer of 5 minutes
// #define WAKE_INTERVAL_SEC 6000000 // wake up timer of 6 seconds for testing


void adc_init();
int read_adc_value(adc1_channel_t channel);
float batteryMonitoring();

#endif