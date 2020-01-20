#include <stdio.h>
#include <sys/stat.h>
#include "main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <unistd.h>

#define CHAR_SIZE 1
#define TAG_LENGTH 12


static const char *TAG = "example";

void handler(){
    printf("Authorised!\n");
}

void app_main(void)
{  
    filesystem_init();
    //Pokrenuti samo prvi put da se izbrise file "/spiffs/RFIDtags.txt"
    //deleteRFIDfile();
    while(1);
   
}

