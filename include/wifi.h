#ifndef WIFI_H
#define WIFI_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/ip4_addr.h"
#include "cJSON.h"
#include "esp_netif_ip_addr.h"
#include <netdb.h>
#include "storage.h"
#include "event.h"
#include "esp_mac.h"
#include "esp_err.h"
#include "esp_check.h"


#define EXAMPLE_ESP_MAXIMUM_RETRY  10
#define SSID "Gestor"
#define CANAL 1
#define PASS 
#define MAXCON 1


typedef enum{
    ON_ERROR,
    ON_CONFIG,
    ON_MAX_RETRY,
}wifi_call_Ap_t;



esp_err_t wifi_init_sta(const char *ssid, const char* password);
void Wifi_stop(void);
esp_err_t Wifi_start(void);
char *wifi_scan(void);
esp_err_t wifi_Ap_call(void);
esp_err_t wifi_init_softap(void);

#endif