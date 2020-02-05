/*==============================================================================================================================================================================*/
/*==============================================================================================================================================================================*/
/*																							SmartBox Fer																		*/
/*==============================================================================================================================================================================*/
/*	This part was made by Karlo Strbad and Ivan Vuger																																*/
/*==============================================================================================================================================================================*/
/* This is project for collage on FER in Zagreb. Mentor is Izv. prof. dr. sc. Hrvoje Dzapo  and with his assisent's Ivan Pavić, mag.ing. and Zrinka Kovačić mag.ing. 			*/
/* This is part for mesure weight with function for taking measure and function for making Tera on scale 																		*/
/*==============================================================================================================================================================================*/

/*==============================================================================================================================================================================*/
/* 																							Include part																		*/
/*==============================================================================================================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/xtensa_api.h"
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "soc/spi_periph.h"
#include "soc/gpio_periph.h"
#include "sdkconfig.h"
#include <rom/ets_sys.h>
#include "load_cell.h"

/*==============================================================================================================================================================================*/
/*																						Define part																				*/
/*==============================================================================================================================================================================*/

#define SPI2_CHANNEL HSPI_HOST
#define SPI3_CHANNEL VSPI_HOST
#define SPI_CLOCK 500000 // Default SPI clock 500KHz
#define SPI_MODE 0		 // Default SPI mode 2*******

#define SPI_CLK_GPIO_2 14  //SCK pin for SPI2
#define SPI_MISO_GPIO_2 12 //MISO pin for SPI2
#define SPI_MOSI_GPIO_2 -1

#define SPI_CLK_GPIO_3 18  //SCK pin for SPI3
#define SPI_MISO_GPIO_3 19 //MISO pin for SPI3
#define SPI_MOSI_GPIO_3 -1

/*==============================================================================================================================================================================*/
/*																						Global variable																			*/
/*==============================================================================================================================================================================*/

esp_err_t ret;

/*==============================================================================================================================================================================*/
/*																			Function's for initialitation SPI2 and SPI3															*/
/*==============================================================================================================================================================================*/

// Initialize the SPI2 device in master mode
spi_device_handle_t SPI2_init(void)
{
	// Configuration for the SPI2 bus
	spi_bus_config_t buscfg_2 = {
		.mosi_io_num = SPI_MOSI_GPIO_2,
		.miso_io_num = SPI_MISO_GPIO_2,
		.sclk_io_num = SPI_CLK_GPIO_2,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1};

	ESP_ERROR_CHECK(spi_bus_initialize(SPI2_CHANNEL, &buscfg_2, 0));

	spi_device_handle_t handle;

	// Configuration for the SPI master interface
	spi_device_interface_config_t devcfg = {
		.command_bits = 0,
		.address_bits = 0,
		.dummy_bits = 0,
		.mode = SPI_MODE,
		.duty_cycle_pos = 0,
		.cs_ena_posttrans = 0,
		.cs_ena_pretrans = 0,
		.clock_speed_hz = SPI_CLOCK,
		.spics_io_num = -1,
		.flags = 0,
		.queue_size = 1,
		.pre_cb = NULL,
		.post_cb = NULL};

	ESP_ERROR_CHECK(spi_bus_add_device(SPI2_CHANNEL, &devcfg, &handle));

	return handle;
}

// Initialize the SPI3 device in master mode
spi_device_handle_t SPI3_init(void)
{
	// Configuration for the SPI3 bus
	spi_bus_config_t buscfg_3 = {
		.mosi_io_num = SPI_MOSI_GPIO_3,
		.miso_io_num = SPI_MISO_GPIO_3,
		.sclk_io_num = SPI_CLK_GPIO_3,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1};

	ESP_ERROR_CHECK(spi_bus_initialize(SPI3_CHANNEL, &buscfg_3, 0));

	spi_device_handle_t handle;

	// Configuration for the SPI master interface
	spi_device_interface_config_t devcfg = {
		.command_bits = 0,
		.address_bits = 0,
		.dummy_bits = 0,
		.mode = SPI_MODE,
		.duty_cycle_pos = 0,
		.cs_ena_posttrans = 0,
		.cs_ena_pretrans = 0,
		.clock_speed_hz = SPI_CLOCK,
		.spics_io_num = -1,
		.flags = 0,
		.queue_size = 1,
		.pre_cb = NULL,
		.post_cb = NULL};

	ESP_ERROR_CHECK(spi_bus_add_device(SPI3_CHANNEL, &devcfg, &handle));

	return handle;
}

/*==============================================================================================================================================================================*/
/*																		Function to make scale start with zero																	*/
/*==============================================================================================================================================================================*/

uint32_t Tera(spi_device_handle_t spi_handle)
{
	uint8_t data[4] = {};
	uint32_t nuliste = 0; //variable for avrage number of scale when os without load
	int i;

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = 8 * 3;
	t.flags = SPI_TRANS_USE_RXDATA;
	t.rx_buffer = data;
	uint32_t rx_data;
	t.tx_buffer = NULL;

	do
	{
		ret = spi_device_transmit(spi_handle, &t);
		assert(ret == ESP_OK);

		// cast to uint32
		rx_data = 0x00000000 | ((uint32_t)t.rx_data[2]) | ((uint32_t)t.rx_data[1] << 8) | ((uint32_t)t.rx_data[0] << 16);

	} while (rx_data>=1188400);

	for (i = 0; i < 5; ++i)
	{
		ret = spi_device_transmit(spi_handle, &t);
		assert(ret == ESP_OK);

		// cast to uint32
		rx_data = 0x00000000 | ((uint32_t)t.rx_data[2]) | ((uint32_t)t.rx_data[1] << 8) | ((uint32_t)t.rx_data[0] << 16);

		ets_delay_us(1000 * 500);
		nuliste += rx_data;
	}

	return nuliste / 5;
}

/*==============================================================================================================================================================================*/
/* 																	Function for reading weight with conversion to kilogram														*/
/*==============================================================================================================================================================================*/

double read_spi(spi_device_handle_t spi_handle, uint32_t tera_spi)
{
	uint8_t data[4] = {};
	double mass;

	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length = 8 * 3;
	t.flags = SPI_TRANS_USE_RXDATA;
	t.rx_buffer = data;
	t.tx_buffer = NULL;

	ret = spi_device_transmit(spi_handle, &t);
	assert(ret == ESP_OK);

	// cast to uint32
	uint32_t rx_data = 0x00000000 | ((uint32_t)t.rx_data[2]) | ((uint32_t)t.rx_data[1] << 8) | ((uint32_t)t.rx_data[0] << 16);

	mass = (rx_data - tera_spi) / 111150.;
	return mass;
}

/**==============================================================================================================================================================================*/
/*																							Master initialitation																*/
/*==============================================================================================================================================================================*/

void init_spi(uint32_t *tera_spi2, uint32_t *tera_spi3, spi_device_handle_t *spi2_handle, spi_device_handle_t *spi3_handle)
{

	*spi2_handle = SPI2_init();
	*spi3_handle = SPI3_init();

	*tera_spi2 = Tera(*spi2_handle); //calling function Tera for making to start from zero
	*tera_spi3 = Tera(*spi3_handle);
}
