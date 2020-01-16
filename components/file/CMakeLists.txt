idf_component_register(SRCS "file.c"
                    INCLUDE_DIRS . 
                    REQUIRES spi_flash
                    REQUIRES fatfs)