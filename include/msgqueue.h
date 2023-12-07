#ifndef MSGQUEUE_H
#define MSGQUEUE_H
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define QUEUE_MAX_LAP 5
#define QUEUE_SIZE 5
#define MAX_PAYLOAD 512
#define MAX_TOPIC 128

typedef enum{
    WS_TX,
    WS_RX,
    MQTT_TX,
    MQTT_RX,
    HTTP_TX,
    HTTP_RX,
    OLED_TX,
    MASTER,
    DIMMER_RX,
    DIMMER_TX,
    CONFIG,
    STORAGE,
}msg_type;

typedef struct msg{ //  estructura de la  cola infinita 
    int count;
    int dest;
    int len_topic;
    char topic[MAX_TOPIC];
    int len_msg;
    char msg[MAX_PAYLOAD];
}msg_queue_t;


esp_err_t queue_send(int dest,const char* payload, const char* topic,TickType_t time);
msg_queue_t queue_receive(int dest,TickType_t time);
esp_err_t queue_start(void);
int queue_load(void);
void queue_reset(void);

#endif