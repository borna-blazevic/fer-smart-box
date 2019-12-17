

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_tls.h"

#define WEB_PORT 1337
#define WEB_URL "192.168.43.134"
#define UNLOCK_MESSAGE "unlock"

static const char *TAG = "TLS";
extern const uint8_t server_root_cert_pem_start[] asm("_binary_public_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[] asm("_binary_public_cert_pem_end");

int tls_init(esp_tls_t **tls, int limit_reconnext)
{
    int ret = 0, i = 0;

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

static void tls_read_task(void *pvParameters)
{
    char buf[512];
    int ret, len;
    esp_tls_t *tls;

    esp_err_t (*handler)(void);
    handler = (esp_err_t(*)(void))pvParameters;

    ret = tls_init(&tls, 0);

    if (ret != 1)
        goto exit;

loop:
    len = sizeof(buf) - 1;
    bzero(buf, sizeof(buf));
    ret = esp_tls_conn_read(tls, (char *)buf, len);

    if (ret == ESP_TLS_ERR_SSL_WANT_WRITE || ret == ESP_TLS_ERR_SSL_WANT_READ)
        goto loop;

    if (ret < 0)
    {
        ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
        goto restart;
    }

    if (ret == 0)
    {
        ESP_LOGI(TAG, "connection closed");
    }

    len = ret;
    ESP_LOGD(TAG, "%d bytes read", len);
    /* Print response directly to stdout as it is read */

    ret = strcmp(UNLOCK_MESSAGE, buf);
    if (ret == 0)
    {
        handler();
    }
    goto loop;

restart:
    esp_tls_conn_delete(tls);
    ret = tls_init(&tls, 0);

    if (ret != 1)
        goto exit;
    else
        goto loop;

exit:
    if (tls != NULL)
        esp_tls_conn_delete(tls); // JSON output doesn't have a newline at end

    ESP_LOGE(TAG, "failed to connect");
}

void tls_read(esp_err_t (*handler)(void))
{
    xTaskCreate(tls_read_task, "tls_read_task", 8192, handler, 5, NULL);
}
void tls_clear_conn(esp_tls_t *tls)
{
    esp_tls_conn_delete(tls);
}

int tls_write(char *message, esp_tls_t *tls)
{
    int ret;

    size_t written_bytes = 0;
    do
    {
        ret = esp_tls_conn_write(tls,
                                 message + written_bytes,
                                 strlen(message) - written_bytes);
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

    return 1;

error:
    ESP_LOGE(TAG, "failed to connect");
    return ret;
}