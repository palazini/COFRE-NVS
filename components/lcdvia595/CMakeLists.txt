idf_component_register(SRCS "lcdvia595.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_wifi
                    REQUIRES esp_netif
                    )