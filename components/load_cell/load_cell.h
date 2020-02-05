#include "driver/spi_master.h"
#include "driver/spi_slave.h"
void init_spi(uint32_t *tera_spi2, uint32_t *tera_spi3, spi_device_handle_t *spi2_handle, spi_device_handle_t *spi3_handle);
double read_spi(spi_device_handle_t spi_handle, uint32_t tera_spi);