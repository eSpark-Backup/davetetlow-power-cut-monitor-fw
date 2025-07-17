#include "cloud_handling.h"

static const char *TAG = "Cloud Handling";

uint8_t DEVICE_MAC_ID[6];
char *create_json_data(bool power_cut, float voltages, uint16_t adc_value)
{
    bool low_battery;
    static char json_data[256];

    if (voltages < 2.04)
        low_battery = true;
    else
        low_battery = false;

    esp_read_mac(DEVICE_MAC_ID, ESP_MAC_WIFI_STA);

    char mac_str[24]; // ESP_34:CD:B0:85:8C:AC
    snprintf(mac_str, sizeof(mac_str), "ESP_%02X:%02X:%02X:%02X:%02X:%02X",
             DEVICE_MAC_ID[0], DEVICE_MAC_ID[1], DEVICE_MAC_ID[2], DEVICE_MAC_ID[3], DEVICE_MAC_ID[4], DEVICE_MAC_ID[5]);

    // create json
    snprintf(json_data, sizeof(json_data),
             "{\"device_id\":\"%s\", \"cutoff_detection\":%s, \"low_battery_indication\":%s, \"ADC Value\":%d , \"Voltages\":%.2f, \"Battery Percentage\":%.0f}",
             mac_str, power_cut ? "true" : "false", low_battery ? "true" : "false", adc_value, voltages, ((voltages - 1.8) / (3.0 - 1.8)) * 100);
    ESP_LOGI(TAG, "%s", json_data);
    return json_data;
}

esp_err_t send_report_to_server(char *json_data)
{
    esp_http_client_config_t config = {
        .url = FLASK_SERVER_URL,
        .method = HTTP_METHOD_POST};

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/json");

    esp_http_client_set_post_field(client, json_data, strlen(json_data));

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI("HTTP", "Response code: %d", esp_http_client_get_status_code(client));
        char buffer[256];
        int len = esp_http_client_read_response(client, buffer, sizeof(buffer) - 1);
        if (len > 0)
        {
            buffer[len] = '\0'; // Null-terminate
            ESP_LOGI("HTTP", "Response: %s", buffer);
        }
    }
    else
    {
        ESP_LOGE("HTTP", "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}
