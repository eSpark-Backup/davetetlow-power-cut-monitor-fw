idf_component_register(SRCS "PowerCutMonitor-fw.c" "monitor_battery.c" "wifi_provisioning.c" "cloud_handling.c" "nvs_store_retrieve.c" "status_led.c"
                    INCLUDE_DIRS "."
                    REQUIRES
                        bt 
                        driver
                        esp_adc
                        nvs_flash
                        esp_http_client
                        esp_wifi
                        json
                    )

