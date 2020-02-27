idf_component_register(SRCS "wifi.c"
                    INCLUDE_DIRS .
                    REQUIRES lwip
                    REQUIRES wpa_supplicant)