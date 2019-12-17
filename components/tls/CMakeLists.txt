idf_component_register(SRCS "tls.c"
                    INCLUDE_DIRS .
                    EMBED_TXTFILES public_cert.pem
                    REQUIRES esp-tls)