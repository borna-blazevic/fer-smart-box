

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "tls.h"

#define WEB_PORT 1337
#define WEB_URL "161.53.64.218"
#define UNLOCK_MESSAGE "unlock"

static const char *TAG = "TLS";
extern const uint8_t server_root_cert_pem_start[] asm("_binary_public_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[] asm("_binary_public_cert_pem_end");

SemaphoreHandle_t xSemaphore = NULL;

int system_tls_init(esp_tls_t **tls, int limit_reconnext)
{
    int ret = 0, i = 0;
    if (xSemaphore == NULL)
    {
        vSemaphoreCreateBinary(xSemaphore);
    }

    while (!limit_reconnext || i < limit_reconnext)
    {
        ESP_LOGI(TAG, "Connecting");
        esp_tls_cfg_t cfg = {
            .cacert_buf = server_root_cert_pem_start,
            .cacert_bytes = server_root_cert_pem_end - server_root_cert_pem_start,
        };

        *tls = esp_tls_init();
        ret = esp_tls_conn_new_sync(WEB_URL, 14, WEB_PORT, &cfg, *tls);

        if (ret == 1)
        {
            ESP_LOGI(TAG, "Connection established...");
            return ret;
        }
        else
        {
            ESP_LOGE(TAG, "return %d", ret);
            ESP_LOGE(TAG, "connection failed...");
            esp_tls_conn_delete(*tls);
            if (limit_reconnext)
                i++;
        }
    }
    return ret;
}

static void tls_heartbeat_task(void *pvParameters)
{
    int ret = 0;
    char message = '1';
    esp_tls_t **tls = (esp_tls_t **)pvParameters;

// See if we can obtain the semaphore.  If the semaphore is not available
// wait 10 ticks to see if it becomes free.
loop:
    do
    {

        vTaskDelay(3000 / portTICK_PERIOD_MS);
        if (xSemaphore != NULL)
        {
            if (xSemaphoreTake(xSemaphore, (TickType_t)10) == pdTRUE)
            {
                ret = esp_tls_conn_write(*tls,
                                         &message,
                                         1);
                xSemaphoreGive(xSemaphore);
            }
            else
            {
                ret = -1;
                goto loop;
            }
        }
        if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE && ret <= 0)
        {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
        }
    } while (1);
}

static void tls_read_task(void *pvParameters)
{
    char buf[3072];
    int ret, len;

    tls_read_arguments *args = (tls_read_arguments *)pvParameters;
loop:
    len = sizeof(buf) - 1;
    bzero(buf, sizeof(buf));

    ret = esp_tls_conn_read(*(args->tls), (char *)buf, len);

    if (ret == ESP_TLS_ERR_SSL_WANT_WRITE || ret == ESP_TLS_ERR_SSL_WANT_READ)
        goto loop;

    if (ret < 0)
    {
        ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
        goto restart;
    }

    if (ret == 0)
    {
        ESP_LOGE(TAG, "connection closed");
    }

    len = ret;
    ESP_LOGE(TAG, "%d bytes read", len);
    args->handler(buf);
    goto loop;

restart:
    if (xSemaphore != NULL)
    {
        if (xSemaphoreTake(xSemaphore, (TickType_t)10) == pdTRUE)
        {
            esp_tls_conn_delete(*(args->tls));
            ret = system_tls_init(args->tls, 0);
            xSemaphoreGive(xSemaphore);

            if (ret != 1)
            {
                goto exit;
            }
            else
            {
                goto loop;
            }
        }
        else
        {
            goto restart;
        }
    }
exit:
    if (xSemaphore != NULL)
    {
        if (xSemaphoreTake(xSemaphore, (TickType_t)10) == pdTRUE)
        {
            if (*(args->tls) != NULL)
                esp_tls_conn_delete(*(args->tls)); // JSON output doesn't have a newline at end
            xSemaphoreGive(xSemaphore);
        }
    }

    ESP_LOGE(TAG, "failed to connect");
}

void tls_read(tls_read_arguments *arguments)
{
    xTaskCreate(tls_read_task, "tls_read_task", 8192, arguments, 5, NULL);
}
void tls_heartbeat(esp_tls_t **tls)
{
    xTaskCreate(tls_heartbeat_task, "tls_heartbeat_task", 10240, tls, 5, NULL);
}
void tls_clear_conn(esp_tls_t *tls)
{
    esp_tls_conn_delete(tls);
}

int tls_write(char *message, esp_tls_t *tls)
{
    int ret = 0;

    size_t written_bytes = 0;

    if (xSemaphore != NULL)
    {
        // See if we can obtain the semaphore.  If the semaphore is not available
        // wait 10 ticks to see if it becomes free.

        do
        {
            if (xSemaphoreTake(xSemaphore, (TickType_t)10) == pdTRUE)
            {
                ret = esp_tls_conn_write(tls,
                                         message + written_bytes,
                                         strlen(message) - written_bytes);
                xSemaphoreGive(xSemaphore);
            }
            else
            {
                ret = -1;
                goto error;
            }

            if (ret >= 0)
            {
                ESP_LOGI(TAG, "%d bytes written", ret);
                written_bytes += ret;
            }
            else if (ret != ESP_TLS_ERR_SSL_WANT_READ && ret != ESP_TLS_ERR_SSL_WANT_WRITE)
            {
                ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
                goto error;
            }
        } while (written_bytes < strlen(message));
    }

    return 1;

error:
    ESP_LOGE(TAG, "failed to connect");
    return ret;
}