idf_component_register(SRCS "ioplaca.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver
                    REQUIRES esp_wifi
                    REQUIRES esp_netif
                    REQUIRES esp_timer
                    )