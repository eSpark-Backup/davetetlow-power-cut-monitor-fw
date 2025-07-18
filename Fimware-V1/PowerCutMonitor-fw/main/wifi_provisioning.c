#include "wifi_provisioning.h"
#include "nvs_store_retrieve.h"
#include "cloud_handling.h"

#include <freertos/event_groups.h>

char *TAG = "BLE-Server";
uint8_t ble_addr_type;

#define MAX_JSON_LEN 512

static char json_buffer[MAX_JSON_LEN];
static int json_index = 0;

void ble_app_advertise(void);

#define MAX_PREPARE_LEN 512

static char prepare_buf[MAX_PREPARE_LEN];
static int prepare_len = 0;

volatile bool wifi_connected = false;

#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t wifi_event_group;
static bool stop_reconnect = false;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI("WIFI", "Connecting to Wi-Fi...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (!stop_reconnect)
        {
            ESP_LOGW("WIFI", "Disconnected. Reconnecting...");
            esp_wifi_connect(); // auto-reconnect
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI("WIFI", "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_connected = true;
    }
}

esp_err_t wifi_connect(const char *ssid, const char *password)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    static bool event_loop_created = false;
    if (!event_loop_created)
    {
        esp_err_t err = esp_event_loop_create_default();
        if (err != ESP_OK && err != ESP_ERR_INVALID_STATE)
        {
            ESP_LOGE(TAG, "Failed to create default event loop: %s", esp_err_to_name(err));
            return err;
        }
        event_loop_created = true;
    }

    static esp_netif_t *sta_netif = NULL;
    if (sta_netif == NULL)
    {
        sta_netif = esp_netif_create_default_wifi_sta();
    }

    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    // esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("WIFI", "Connecting to %s...", ssid);

    // Wait until connected (timeout 10 seconds)
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
                                           pdFALSE, pdTRUE, pdMS_TO_TICKS(10000));

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI("WIFI", "Connected successfully.");
        return ESP_OK;
    }
    else
    {
        ESP_LOGE("WIFI", "Failed to connect to Wi-Fi within timeout.");
        stop_reconnect = true;
        esp_wifi_disconnect();
        esp_wifi_stop();
        vEventGroupDelete(wifi_event_group);
        esp_wifi_deinit();
        return ESP_FAIL;
    }
}

void parse_json(const char *json_str)
{
    cJSON *root = cJSON_Parse(json_str);
    if (!root)
    {
        printf("Error parsing JSON\n");
        return;
    }

    cJSON *ssid = cJSON_GetObjectItem(root, "ssid");
    cJSON *password = cJSON_GetObjectItem(root, "password");

    if (cJSON_IsString(ssid) && cJSON_IsString(password))
    {
        ESP_LOGI("WIFI_CRED", " SSID: %s\n", ssid->valuestring);
        ESP_LOGI("WIFI_CRED", " Password: %s\n", password->valuestring);

        if (wifi_connect(ssid->valuestring, password->valuestring) == ESP_OK)
        {
            ESP_LOGI("WIFI", "Wifi Connected****************");
            char *json_data;
            json_data = create_json_data_2();
            ESP_LOGI("JSON", "%s", json_data);
            send_report_to_server(json_data);

            if (nvs_write_string("wifi_creds", "ssid", ssid->valuestring) == ESP_OK)
            {
                ESP_LOGI(TAG, "Successfully write string to NVS");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to write string to NVS");
            }
            if (nvs_write_string("wifi_creds", "pass", password->valuestring) == ESP_OK)
            {
                ESP_LOGI(TAG, "Successfully write string to NVS");
            }
            else
            {
                ESP_LOGE(TAG, "Failed to write string to NVS");
            }
        }
        else
        {
            ESP_LOGW("WIFI", "Wifi Not Connected****************");
        }
    }
    else
    {
        printf(" Missing or invalid fields\n");
    }

    cJSON_Delete(root);
}
// }

int device_write(uint16_t conn_handle, uint16_t attr_handle,
                 struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const struct os_mbuf *om = ctxt->om;

    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
    case BLE_GATT_ACCESS_OP_WRITE_DSC:
    {
        int len = OS_MBUF_PKTLEN(om); // Total length

        prepare_len = 0;
        int rc = os_mbuf_copydata(om, 0, len, prepare_buf);
        if (rc != 0)
        {
            printf("Error copying data\n");
            return BLE_ATT_ERR_UNLIKELY;
        }
        prepare_buf[len] = '\0';
        prepare_len = len;

        printf(" Final Write Received (%d bytes): %s\n", prepare_len, prepare_buf);

        stop_reconnect = false;
        parse_json(prepare_buf);

        break;
    }

    default:
        printf("Unknown write operation\n");
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}
// Read data from ESP32 defined as server
int device_read(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "Data from the server", strlen("Data from the server"));

    return 0;
}

const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180), // Define UUID for device type
     .characteristics = (struct ble_gatt_chr_def[]){
         //  {.uuid = BLE_UUID16_DECLARE(0xFEF4), // Define UUID for reading
         //   .flags = BLE_GATT_CHR_F_READ,
         //   .access_cb = device_read},
         {.uuid = BLE_UUID16_DECLARE(0xCDAD), // Define UUID for writing
          .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
          .access_cb = device_write},
         {0}}},
    {0}};

// BLE event handling
int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    // Advertise if connected
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            ble_app_advertise();
        }
        break;
    // Advertise again after completion of the event
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT DISCONNECTED");
        ble_app_advertise();
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    default:
        break;
    }
    return 0;
}

void ble_app_advertise(void)
{
    // GAP - device name definition
    struct ble_hs_adv_fields fields;
    const char *device_name;
    memset(&fields, 0, sizeof(fields));
    device_name = ble_svc_gap_device_name(); // Read the BLE device name
    fields.name = (uint8_t *)device_name;
    fields.name_len = strlen(device_name);
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);

    // GAP - device connectivity definition
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND; // connectable or non-connectable
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN; // discoverable or non-discoverable
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

// The application
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type); // Determines the best address type automatically
    ble_app_advertise();                     // Define the BLE connection
}

// The infinite task
void host_task(void *param)
{
    nimble_port_run(); // This function will return only when nimble_port_stop() is executed
}
