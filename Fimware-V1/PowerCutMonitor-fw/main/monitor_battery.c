#include "monitor_battery.h"
static const char *TAG = "Battery Monitoring";

void adc_init()
{
    adc1_config_width(EXAMPLE_ADC_BIT_WIDTH);
    adc1_config_channel_atten(EXAMPLE_ADC1_CHAN0, EXAMPLE_ADC_ATTEN);
}

int read_adc_value(adc1_channel_t channel)
{
    // return adc1_get_raw(channel);
    int adc_reading = 0;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        adc_reading += adc1_get_raw(channel);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return adc_reading / NUM_SAMPLES;
}

float batteryMonitoring(int *adcc_Value)
{
    int adc_value;
    float voltages = 0.0;

    adc_value = read_adc_value(EXAMPLE_ADC1_CHAN0);
    ESP_LOGI(TAG, "ADC Value: %d", adc_value);
    *adcc_Value = adc_value;
    voltages = (adc_value / 4095.0) * 3.0;
    ESP_LOGI(TAG, "voltages: %.2f", voltages);

    return voltages;
}
