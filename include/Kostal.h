#ifndef KOSTAL_H
#define KOSTAL_H
#include "esp_http_client.h"
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "xread.h"
#include <stdio.h>

esp_http_client_handle_t  http_begin(const char *url);
float Kostal_requests(esp_http_client_handle_t client);

#endif


