#ifndef WIFI_PROVISIONING_H
#define WIFI_PROVISIONING_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "esp_wifi.h"
#include "esp_system.h"
// #include "sdkconfig.h"
#include "cJSON.h"

extern const struct ble_gatt_svc_def gatt_svcs[];
extern volatile bool wifi_connected;

void ble_app_advertise(void);
esp_err_t wifi_connect(const char *ssid, const char *password);
void parse_json(const char *json_str);
int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg);
int ble_gap_event(struct ble_gap_event *event, void *arg);
void ble_app_on_sync(void);
void host_task(void *param);
esp_err_t init_ble();

#endif