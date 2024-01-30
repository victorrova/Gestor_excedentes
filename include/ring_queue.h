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
    void *topic;
    int len_msg;
    void *msg;
}msg_queue_t;


esp_err_t queue_send(int dest,const char* payload, const char* topic,TickType_t time);
msg_queue_t queue_receive(int dest,TickType_t time);
esp_err_t queue_start(void);
int queue_load(void);
void queue_reset(void);
esp_err_t queue_receive_instat(int dest,msg_queue_t msg);
#endif