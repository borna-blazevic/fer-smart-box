#include "esp_tls.h"

typedef struct{
    esp_err_t (*handler)(char *message);
    esp_tls_t **tls;
}tls_read_arguments;

void tls_read(tls_read_arguments *arguments);
int tls_init(esp_tls_t **tls, int limit_reconnext);
int tls_write(char *message, esp_tls_t *tls);
void tls_clear_conn(esp_tls_t *tls);
void tls_heartbeat(esp_tls_t **tls);
