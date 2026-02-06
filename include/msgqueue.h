/*Cola en anillo para la comunicación entre tareas con autolimpieza de mensajes huerfanos*/


#ifndef MSGQUEUE_H
#define MSGQUEUE_H
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "config.h"
#include "esp_log.h"



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
}msg_destination;

typedef enum{
    NONE = -1,
    DIMMER_LEVEL,
    TEMP_VALUE,
    POWER_VALUE,
    PID_KP,
    PID_KI,
    PID_KD,
    PID_MIN,
    PID_MAX,
    MQTT_SUB,
    MQTT_PUB,
}msg_type;


typedef struct msg{             //estructura de la  cola en anillo 
    int count;                  // llamadas sin enncontrar destino 
    msg_destination dest;                   // destino ( msg_type)        
    msg_type type;
    void *payload;
}msg_queue_t;


esp_err_t queue_send(msg_destination dest,void* payload, msg_type type,TickType_t time);
esp_err_t queue_receive(msg_destination dest,TickType_t time,msg_queue_t *msg);
esp_err_t queue_start(void);
int queue_load(void);
void queue_reset(void);
esp_err_t queue_receive_instat(msg_destination dest,msg_queue_t*msg);


#endif