idf_component_register(SRCS "PowerCutMonitor-fw.c" "monitor_battery.c" "wifi_provisioning.c" "cloud_handling.c" "nvs_store_retrieve.c"
                    INCLUDE_DIRS "."
                    REQUIRES
                        bt 
                        driver
                        nvs_flash
                        esp_http_client
                        esp_wifi
                        json
                    #     esp_hw_support
                    #     driver    
                    )
