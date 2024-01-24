#ifndef MQTT_H
#define MQTT_H
#include <string.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "storage.h"
#include "event.h"
#include "mqtt_client.h"
#include "helper.h"
#include "wifi.h"
#include "msgqueue.h"



esp_err_t mqtt_init(void);
esp_err_t mqtt_publish(char * payload,int payload_len, char* topic);
esp_err_t queue_to_mqtt_publish(msg_queue_t msg);
#endif