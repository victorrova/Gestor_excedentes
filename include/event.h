#ifndef EVENT_H
#define EVENT_H

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_MODE          BIT3
#define MQTT_CONNECT       BIT4
#define MQTT_DISCONNECT    BIT5
#define MQTT_ON_MESSAGE    BIT6


esp_err_t Event_init(void);
#endif