#include "monitor_battery.h"
static const char *TAG = "Battery Monitoring";

// void configure_gpios(void)
// {
//     // Configure output pins
//     gpio_config_t io_conf_out = {
//         .pin_bit_mask = (1ULL << GPIO_OUTPUT),
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_up_en = 0,
//         .pull_down_en = 0,
//         .intr_type = GPIO_INTR_DISABLE,
//     };
//     gpio_config(&io_conf_out);
// }

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

float batteryMonitoring()
{
    int adc_value;
    float voltages = 0.0;

    adc_value = read_adc_value(EXAMPLE_ADC1_CHAN0);
    ESP_LOGI(TAG, "ADC Value: %d", adc_value);
    voltages = (adc_value / 4095.0) * 3.5;
    voltages = voltages * 3.1;       // external Attenuation 
    ESP_LOGI(TAG, "voltages: %.2f", voltages);

    return voltages;
}
