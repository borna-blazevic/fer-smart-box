/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

/**
 * This is an example which echos any data it receives on UART1 back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define CORRECT "0F0194B9CAE9"

#define ECHO_TEST_TXD  (GPIO_NUM_1)
#define ECHO_TEST_RXD  (GPIO_NUM_3)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)


//T1 0F0194B7537E
//T2 0F01949BF0F1
static const char *TAG = "RFID";
#define BUF_SIZE (256)

int process(uint8_t* buffer, uint8_t *output){
  int i,j;
  for(i=0;i<12;i++){
    if(buffer[i] == '0' && buffer[i+1] == 'F' && buffer[i+2] == '0' && buffer[i+3] == '1' && buffer[i+4] == '9' && buffer[i+5] == '4' && i <= 10){
      for(j=0;j<12;j++){
        output[j] = buffer[i+j];
      }
      return 1;
    }
  }
  return 0;
}

static void read_rfid_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);

    void (*handler)(char *rfid) = (void(*)(char *rfid))arg;

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    uint8_t rfid[12];
    int ret;


    while (1) {
        // Read data from the UART
        uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 20 / portTICK_RATE_MS);
        ret = process(data, rfid);
        if(ret){
                handler((char *)rfid);
        }
        memset(data,0,BUF_SIZE);
        // Write data back to the UART

    }
}

void read_rfid(void (*handler)(void))
{
    xTaskCreate(read_rfid_task, "read_rfid_task", 8192, handler, 2, NULL);
}


