#include "nvs_store_retrieve.h"

// #include <stdlib.h>

// #include <stdlib.h>
// #include <string.h>
// #include "esp_check.h"
// #include "esp_log.h"

// #include "nvs.h"
// #include "nvs_flash.h"

static const char *TAG = "NVS_STORE_RETRIEVE";

esp_err_t nvs_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

static esp_err_t esp32_nvs_open(const char *namespace, nvs_open_mode_t open_mode, nvs_handle_t *nvs_handle)
{
    esp_err_t err = nvs_open(namespace, open_mode, nvs_handle);
    if (err != ESP_OK)
    {
        // ESP_LOGE(TAG, "%s(): Error opening NVS namespace %s: %d (%s)!", __func__, namespace, err, esp_err_to_name(err));
    }
    return err;
}

static esp_err_t esp32_nvs_read(const char *namespace, const char *key, nvs_type_t type_value,
                                void *value, size_t length)
{
    if (namespace == NULL)
    {
        // ESP_LOGE(TAG, "%s(): Failed to read value: namespace is NULL!", __func__);
        return ESP_ERR_INVALID_ARG;
    }
    if (key == NULL)
    {
        // ESP_LOGE(TAG, "%s(): Failed to read value: key is NULL!", __func__);
        return ESP_ERR_INVALID_ARG;
    }
    if (type_value != NVS_TYPE_STR)
    {
        if (value == NULL)
        {
            // ESP_LOGE(TAG, "%s(): Failed to read NULL value!", __func__);
            return ESP_ERR_INVALID_ARG;
        }
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = esp32_nvs_open(namespace, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    switch (type_value)
    {
    case NVS_TYPE_STR:
        size_t required_string_size = 0;
        err = nvs_get_str(nvs_handle, key, NULL, &required_string_size);

        if (err == ESP_OK)
        {
            *(char **)value = malloc(required_string_size);
            if (*(char **)value != NULL)
            {
                err = nvs_get_str(nvs_handle, key, *(char **)value, &required_string_size);
            }
            else
            {
                err = ESP_ERR_NO_MEM;
                // ESP_LOGE(TAG, "%s(): Failed to allocate memory", __func__);
            }
        }
        break;
    default:
        err = ESP_ERR_NVS_TYPE_MISMATCH;
        break;
    }

    switch (err)
    {
    case ESP_OK:
        switch (type_value)
        {
        case NVS_TYPE_STR:
            // ESP_LOGI(TAG, "Successfully read string from NVS %s.%s: %s", namespace, key, *(char**)value);
            break;
        default:
#if CONFIG_LOG_MAXIMUM_LEVEL >= CONFIG_LOG_DEFAULT_LEVEL_INFO
            char *string_value = value;
            if (string_value != NULL)
            {
                // ESP_LOGI(TAG, "Successfully read value from NVS %s.%s: %s", namespace, key, string_value);
                free(string_value);
            }
#endif // CONFIG_LOG_MAXIMUM_LEVEL
            break;
        }
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        // ESP_LOGW(TAG, "Value %s.%s is not initialized yet", namespace, key);
        break;
    default:
        // ESP_LOGE(TAG, "Failed to read from NVS %s.%s: %d (%s)", namespace, key, err, esp_err_to_name(err));
        break;
    }

    nvs_close(nvs_handle);
    return err;
}

static esp_err_t esp32_nvs_write(const char *namespace, const char *key, nvs_type_t type_value,
                                 const void *value, size_t length)
{
    if (namespace == NULL)
    {
        ESP_LOGE(TAG, "%s(): Failed to write value: namespace is NULL!", __func__);
        return ESP_ERR_INVALID_ARG;
    }
    if (key == NULL)
    {
        ESP_LOGE(TAG, "%s(): Failed to write value: key is NULL!", __func__);
        return ESP_ERR_INVALID_ARG;
    }
    if (value == NULL)
    {
        ESP_LOGE(TAG, "%s(): Failed to write NULL value!", __func__);
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = esp32_nvs_open(namespace, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK)
    {
        return err;
    }

    switch (type_value)
    {
    case NVS_TYPE_STR:
        err = nvs_set_str(nvs_handle, key, (char *)value);
        break;
    default:
        err = ESP_ERR_NVS_TYPE_MISMATCH;
        break;
    }

    if (err == ESP_OK)
    {
        err = nvs_commit(nvs_handle);
        if (err == ESP_OK)
        {
            switch (type_value)
            {
            case NVS_TYPE_STR:
                // ESP_LOGI(TAG, "Successfully write string to NVS %s.%s: %s", namespace, key, value);
                break;           
            default:
#if CONFIG_LOG_MAXIMUM_LEVEL >= CONFIG_LOG_DEFAULT_LEVEL_INFO
                char *string_value = value;
                if (string_value != NULL)
                {
                    // ESP_LOGI(TAG, "Successfully write value to NVS %s.%s: %s", namespace, key, string_value);
                    free(string_value);
                }
#endif // CONFIG_LOG_MAXIMUM_LEVEL
                break;
            }
        }
    }
    else
    {
        // ESP_LOGE(TAG, "Failed to write to NVS %s.%s: %d (%s)!", namespace, key, err, esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
    return err;
}

esp_err_t nvs_write_string(const char *namespace, const char *key, const char *value)
{
    return esp32_nvs_write(namespace, key, NVS_TYPE_STR, value, 0);
}

esp_err_t nvs_read_string(const char *namespace, const char *key, void *out_value)
{
    return esp32_nvs_read(namespace, key, NVS_TYPE_STR, out_value, 0);
}

// void app_main(void)
// {
//     ESP_ERROR_CHECK(nvs_init());

//     char *ssid = "eSpark WIFI";
//     char *passsword = "43756880";
//     if (nvs_write_string("namespace_1", "ssid", ssid) == ESP_OK)
//     {
//         ESP_LOGI(TAG, "Successfully write string to NVS");
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Failed to write string to NVS");
//     }

//     if (nvs_write_string("namespace_1", "pass", passsword) == ESP_OK)
//     {
//         ESP_LOGI(TAG, "Successfully write string to NVS");
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Failed to write string to NVS");
//     }

//     char *ssid_2 = NULL;
//     if (nvs_read_string("namespace_1", "ssid", &ssid_2) == ESP_OK)
//     {
//         ESP_LOGI(TAG, "Successfully read string from NVS: %s", ssid_2);
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Failed to read string from NVS");
//     }

//     char *passsword_2 = NULL;
//     if (nvs_read_string("namespace_1", "pass", &passsword_2) == ESP_OK)
//     {
//         ESP_LOGI(TAG, "Successfully read string from NVS: %s", passsword_2);
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Failed to read string from NVS");
//     }
//     // IMPORTANT NOTE!: This applies ONLY to strings. Remember to delete the pointer to avoid a memory leak.
//     free(read_string);

// }