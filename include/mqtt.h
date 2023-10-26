#ifndef MQTT_H
#define MQTT_H

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "storage.h"
#include "event.h"
#include "mqtt_client.h"
#include "helper.h"


typedef struct msg_event{
    int topic_len;
    char* topic;
    int payload_len;
    char* payload;
}msg_t;

esp_err_t mqtt_init(void);

#endif