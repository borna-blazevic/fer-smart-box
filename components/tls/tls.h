#include "esp_tls.h"
void tls_read(esp_err_t (*handler)(void));
int tls_init(esp_tls_t **tls, int limit_reconnext);
int tls_write(char *message, esp_tls_t *tls);
void tls_clear_conn(esp_tls_t *tls);