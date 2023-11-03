#ifndef MSGQUEUE_H
#define MSGQUEUE_H
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define QUEUE_MAX_LAP 5
#define QUEUE_SIZE 5


typedef enum{
    WS_TX,
    WS_RX,
    MQTT_TX,
    MQTT_RX,
    HTTP_TX,
    HTTP_RX,
    OLED_TX,
    MASTER,
    DIMMER,
    CONFIG,
}msg_type;

typedef struct msg{ //  estructura de la  cola infinita 
    int count;
    int dest;
    int len_topic;
    char topic[128];
    int len_msg;
    char msg[512];
}msg_queue_t;


esp_err_t queue_send(int dest,const char* payload, const char* topic,TickType_t time);
msg_queue_t queue_receive(int dest,TickType_t time);
esp_err_t queue_start(void);
int queue_load(void);
void queue_reset(void);

#endif