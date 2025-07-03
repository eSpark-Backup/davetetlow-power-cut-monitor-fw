#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
// #include "esp_adc_cal.h"

#define POWER_GPIO 5
#define BATT_GPIO 4

#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_4       // GPIO4 (ADC1_CH4)
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_6      // ADC_ATTEN_DB_11 // Attenuation for higher voltage range
#define EXAMPLE_ADC_BIT_WIDTH ADC_WIDTH_BIT_12 // 12-bit resolution
#define NUM_SAMPLES 150 
#define WAKE_INTERVAL_SEC 10             //1800000000// wake up timer of 10 sec


int capacitySamples[NUM_SAMPLES];
int sampleIndex = 0;
bool bufferFilled = false;

static const char *TAG = "example";

// Voltage-to-capacity lookup table
typedef struct {
  float voltage;
  int capacity;
}VoltageCapacity;

VoltageCapacity battTable[] = {
    {3.00, 100},
    {2.94, 95},
    {2.88, 90},
    {2.82, 85},
    {2.76, 80},
    {2.70, 75},
    {2.64, 70},
    {2.58, 65},
    {2.52, 60},
    {2.46, 55},
    {2.40, 50},
    {2.34, 45},
    {2.28, 40},
    {2.22, 35},
    {2.16, 30},
    {2.10, 25},
    {2.04, 20},
    {1.98, 15},
    {1.92, 10},
    {1.86, 5},
    {1.80, 0}
};


static void configure_GPIO(void)
{
    // ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    /* Set the GPIO as a push/pull input */
    gpio_reset_pin(POWER_GPIO);
    gpio_set_direction(POWER_GPIO, GPIO_MODE_INPUT);
}
static void adc_init()
{
    adc1_config_width(EXAMPLE_ADC_BIT_WIDTH);
    adc1_config_channel_atten(EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN);
}

int read_adc_value(adc1_channel_t channel)
{
    // return adc1_get_raw(channel);
        int adc_reading = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        adc_reading += adc1_get_raw(channel);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return adc_reading / NUM_SAMPLES;
}

int batteryMonitoring(){
    int adc_value;
    float voltages = 0.0;
    
    adc_value = read_adc_value(EXAMPLE_ADC1_CHAN0);
    ESP_LOGI(TAG, "ADC Value: %d", adc_value);

    voltages = (adc_value / 4095.0) * 3.0;
    ESP_LOGI(TAG, "volatges: %.2f", voltages);

    if (voltages > 3.0) voltages = 3.0;
    if (voltages < 1.8) voltages = 1.8;

    int capacity = 0;
    for (int i = 0; i < sizeof(battTable) / sizeof(battTable[0]) - 1; i++) {
      if (voltages >= battTable[i + 1].voltage && voltages <= battTable[i].voltage) {
        float slope = (battTable[i].capacity - battTable[i + 1].capacity) /
                      (battTable[i].voltage - battTable[i + 1].voltage);
        capacity = battTable[i + 1].capacity +
                   (voltages - battTable[i + 1].voltage) * slope;
        break;
      }
    }

    capacitySamples[sampleIndex] = capacity;
    sampleIndex++;
    if (sampleIndex >= NUM_SAMPLES) {
      sampleIndex = 0;
      bufferFilled = true;
    }

    // Calculate average
    int sampleCount = bufferFilled ? NUM_SAMPLES : sampleIndex;
    int sum = 0;
    for (int i = 0; i < sampleCount; i++) {
      sum += capacitySamples[i];
    }
    int avgCapacity = sum / sampleCount;

    return avgCapacity;
}

void app_main(void)
{
    configure_GPIO();
    adc_init();
    int percent;
    while (1)
    {
        percent = batteryMonitoring();
        ESP_LOGI(TAG, "Battery: %d%%", percent);

        int level = gpio_get_level(POWER_GPIO);
        ESP_LOGI(TAG, "GPIO level: %d", level);

        vTaskDelay(pdMS_TO_TICKS(500));

        if (level == 1)
        {
            ESP_LOGI(TAG, "GPIO is HIGH, continuing normal operation...");
           
            /*need to implemet cloud service*/


        }
        else if(percent <= 20){
             /*need to implemet cloud service*/


        }
        else
        {
            ESP_LOGI(TAG, "GPIO is LOW, enter  sleep mode");
            // esp_deep_sleep_enable_gpio_wakeup(1 << POWER_GPIO, 1);
            // esp_light_sleep_start();
            esp_sleep_enable_timer_wakeup(WAKE_INTERVAL_SEC * 1000000ULL);
            vTaskDelay(pdMS_TO_TICKS(500));
            esp_deep_sleep_start();
            ESP_LOGI(TAG, "Wake up from sleep mode");
        }
    }
}
