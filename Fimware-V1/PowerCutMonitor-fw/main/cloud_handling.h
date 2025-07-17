#ifndef CLOUD_HANDLING_H    
#define CLOUD_HANDLING_H

#include <stdio.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <esp_http_client.h>

extern uint8_t DEVICE_MAC_ID[6];
#define SSID "eSpark Wifi"
#define PASSWORD "47356880"

#define FLASK_SERVER_URL "http://192.168.18.21:5000/data"

char *create_json_data(bool power_cut, float voltages, uint16_t adc_value);
esp_err_t wifi_connect();
esp_err_t send_report_to_server(char *json_data);
#endif