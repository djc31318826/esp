#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp32s3/spiram.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

extern httpd_handle_t handle;
#define MAX_CLIENTS 13