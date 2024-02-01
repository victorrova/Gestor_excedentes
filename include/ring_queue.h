#ifndef RING_QUEUE_H
#define RING_QUEUE_H
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "config.h"


typedef enum{
    WS_TX,
    WS_RX,
    MQTT_TX,
    MQTT_RX,
    OLED_TX,
    MASTER,
    DIMMER_RX,
    DIMMER_TX,
    CONTROL,
}msg_type;

typedef struct msg{ //  estructura de la  cola infinita 
    int count;
    int dest;
    int len_topic;
    char topic[MAX_TOPIC];
    int len_msg;
    char msg[MAX_PAYLOAD];
}msg_queue_t;

esp_err_t xqueue_send(int dest,const char* payload, const char* topic,TickType_t time);
esp_err_t xqueue_receive(int dest,TickType_t time,msg_queue_t *msg);
esp_err_t xqueue_start(void);
int xqueue_load(void);
void xqueue_reset(void);
esp_err_t xqueue_receive_instat(int dest,msg_queue_t msg);
#endif