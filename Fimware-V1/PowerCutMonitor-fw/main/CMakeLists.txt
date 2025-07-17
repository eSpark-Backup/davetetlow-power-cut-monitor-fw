idf_component_register(SRCS "wifi_provisioning.c" "PowerCutMonitor-fw.c" "monitor_battery.c" "cloud_handling.c" "wifi_provisioning.c"
                    INCLUDE_DIRS "."
                    REQUIRES
                    #     esp_hw_support
                    #     driver    
                    )
