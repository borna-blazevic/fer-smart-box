#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rfid.h"
#include "wifi.h"
#include "http.h"
#include "tls.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_tls.h"
#include "rfid.h"
#include "file.h"
static const char *TAG = "MAIN";

void rfid_handler(char *rfid)
{
    ESP_LOGI(TAG, "RFID: %s", rfid);
}


esp_err_t tls_handler(void)
{
    ESP_LOGI(TAG, "Cabinet unlocked");
    return 0;
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    tls_read_arguments args;
    esp_tls_t *conn;
    int number;
    char buffer[20];
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    filesystem_init();
    wifi_connect();
    tls_init(&conn, 0);
    args.handler = tls_handler;
    args.tls = &conn;
    tls_read(&args);
    read_rfid(rfid_handler);

    while (1)
    {
        number = esp_random() % 150;
        itoa(number, buffer, 10);
        ret = tls_write(buffer, conn);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    //http_get_url("http://google.hr", http_event_handler);
    ESP_LOGI("connected", "connected");
    while (1)
        ;
}
