idf_component_register(SRCS "http.c"
                    INCLUDE_DIRS .
                    REQUIRES esp_http_client
                    REQUIRES esp-tls)