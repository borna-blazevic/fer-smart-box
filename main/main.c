#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "json.c"
static const char *TAG = "MAIN";

esp_tls_t *conn;
void rfid_handler(char *rfid)
{
    int ret = 0;
    char *json_message;
    message_template message;
    ret = fileFind((uint8_t *)rfid);
    if (ret == -1)
    {
        message.type = NEW;
        strcpy(message.data[0], rfid);
        cToJSON(message, &json_message);
        ret = tls_write(json_message, conn);
    }
    else
    {
        ESP_LOGI(TAG, "Unlocked");
    }
}

esp_err_t tls_handler(char *message)
{
    ESP_LOGI(TAG, "message recieved %s", message);
    const cJSON *type = NULL;
    const cJSON *arg1 = NULL;
    const cJSON *arg2 = NULL;

    cJSON *message_json = cJSON_Parse(message);
    if (message_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            ESP_LOGI(TAG, "Error before: %s\n", error_ptr);
        }
        goto end;
    }

    type = cJSON_GetObjectItemCaseSensitive(message_json, "type");

    if (cJSON_IsNumber(type))
    {
        switch (type->valueint)
        {
        case 0:
            ESP_LOGI(TAG, "UNLOCK command");
            break;
        case 1:
            ESP_LOGI(TAG, "ADD command");
            arg1 = cJSON_GetObjectItemCaseSensitive(message_json, "RFID");
            if (cJSON_IsString(arg1) && (arg1->valuestring != NULL))
            {
                ESP_LOGI(TAG, "ADD %s", arg1->valuestring);
                fileWrite((uint8_t *)arg1->valuestring, 0, SEEK_END);
            }
            read_file();
            break;
        case 2:
            ESP_LOGI(TAG, "REMOVE command");
            arg1 = cJSON_GetObjectItemCaseSensitive(message_json, "RFID");
            if (cJSON_IsString(arg1) && (arg1->valuestring != NULL))
            {
                ESP_LOGI(TAG, "REMOVE %s", arg1->valuestring);
                fileDelete((uint8_t *)arg1->valuestring);
            }
            read_file();
            break;
        case 3:
            ESP_LOGI(TAG, "UPDATE command");
            arg1 = cJSON_GetObjectItemCaseSensitive(message_json, "RFID");
            if (cJSON_IsString(arg1) && (arg1->valuestring != NULL))
            {

                ESP_LOGI(TAG, "arg 1 %s", arg1->valuestring);
                arg2 = cJSON_GetObjectItemCaseSensitive(message_json, "RFID_old");
                if (cJSON_IsString(arg2) && (arg2->valuestring != NULL))
                {

                    ESP_LOGI(TAG, "UPDATE command");
                    ESP_LOGI(TAG, "UPDATE %s %s", arg2->valuestring, arg1->valuestring);
                    fileDelete((uint8_t *)arg2->valuestring);
                    fileWrite((uint8_t *)arg1->valuestring, 0, SEEK_END);
                }
            }
            read_file();
            break;

        case 6:
            ESP_LOGI(TAG, "REPLACE_FILE command");
            arg1 = cJSON_GetObjectItemCaseSensitive(message_json, "file");
            if (cJSON_IsString(arg1) && (arg1->valuestring != NULL))
            {
                ESP_LOGI(TAG, "arg 1 %s", arg1->valuestring);
                arg2 = cJSON_GetObjectItemCaseSensitive(message_json, "length");
                if (cJSON_IsNumber(arg2))
                    {
                        ESP_LOGI(TAG, "arg 2 %d", arg2->valueint);
                        ESP_LOGI(TAG, "REPLACE_FILE command");
                        ESP_LOGI(TAG, "REPLACE_FILE %s %d", arg1->valuestring, arg2->valueint);
                        replaceFile(arg1->valuestring, arg2->valueint);
                    }
                // fileDelete((uint8_t *)arg2->valuestring);
                // fileWrite((uint8_t *)arg1->valuestring, 0, SEEK_END);
            }
            read_file();
            break;

        default:
            break;
        }
    }
end:
    cJSON_Delete(message_json);
    return 0;
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    tls_read_arguments args;
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
    tls_heartbeat(&conn);
    tls_read(&args);
    read_rfid(rfid_handler);

    while (1)
    {
    }

    //http_get_url("http://google.hr", http_event_handler);
    ESP_LOGI("connected", "connected");
    while (1)
        ;
}
