idf_component_register(SRCS "PowerCutMonitor-fw.c" "monitor_battery.c" "cloud_handling.c"
                    INCLUDE_DIRS "."
                    REQUIRES
                    #     esp_hw_support
                    #     driver    
                    )
