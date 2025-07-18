#ifndef NVS_STORE_RETRIEVE
#define NVS_STORE_RETRIEVE

#include <stdlib.h>

#include <stdlib.h>
#include <string.h>
#include "esp_check.h"
#include "esp_log.h"

#include "nvs.h"
#include "nvs_flash.h"

esp_err_t nvs_init(void);
static esp_err_t esp32_nvs_open(const char *namespace, nvs_open_mode_t open_mode, nvs_handle_t *nvs_handle);
static esp_err_t esp32_nvs_read(const char *namespace, const char *key, nvs_type_t type_value,void *value, size_t length);
static esp_err_t esp32_nvs_write(const char *namespace, const char *key, nvs_type_t type_value, const void *value, size_t length);
esp_err_t nvs_write_string(const char *namespace, const char *key, const char *value);
esp_err_t nvs_read_string(const char *namespace, const char *key, void *out_value);


#endif