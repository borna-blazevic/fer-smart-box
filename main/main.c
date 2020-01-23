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
#include "driver/gpio.h"
static const char *TAG = "MAIN";


#define GPIO_OUTPUT_IO    15
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO)


#define GPIO_INPUT_IO     16
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO) 


#define ESP_INTR_FLAG_DEFAULT 0


esp_tls_t *conn;

uint32_t *tera_spi2, *tera_spi3;
spi_device_handle_t *spi2_handle, *spi3_handle;

char last_user[12];


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    float mass1,mass2;
    uint32_t gpio_num = (uint32_t) arg;

    mass3=read_spi(*spi3_handle,*tera_spi3);
    mass2=read_spi(*spi2_handle,*tera_spi2);

    
}

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
        strcpy(last_user, rfid);
    }
    else
    {
        ESP_LOGI(TAG, "Unlocked");

        gpio_set_level(GPIO_OUTPUT_IO, 1);
        vTaskDelay(3000 / portTICK_RATE_MS);
        gpio_set_level(GPIO_OUTPUT_IO, 0);
    }
}

void init_gpio_output()
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}
void init_gpio_input()
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_IO, gpio_isr_handler, (void*) GPIO_INPUT_IO);
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
    init_spi(tera_spi2,tera_spi3, spi2_handle, spi3_handle);

    while (1)
    {
    }
}
