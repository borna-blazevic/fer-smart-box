#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rfid.h"

void rfid_handler(){
    printf("Authorised!\n");
}

void app_main(void)
{
    printf("Hello world!\n");
    read_rfid(rfid_handler);
    while (1)
        ;
}
