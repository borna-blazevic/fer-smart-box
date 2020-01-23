void init_spi(uint32_t *tera_spi2, uint32_t *tera_spi3);
float read_spi(spi_device_handle_t spi_handle, uint32_t tera_spi);
uint32_t Tera(spi_device_handle_t spi_handle);
spi_device_handle_t SPI2_init(void)
spi_device_handle_t SPI3_init(void);