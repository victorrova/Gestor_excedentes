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
#include "helper.h"



/*posibles direcciones para la cola en anillo*/
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

typedef struct msg{             //estructura de la  cola en anillo 
    int count;                  // llamadas sin enncontrar destino 
    int dest;                   // destino ( msg_type)
    int len_topic;              
    char topic[MAX_TOPIC];
    int len_msg;
    char msg[MAX_PAYLOAD];
}msg_queue_t;


esp_err_t queue_send(int dest,const char* payload, const char* topic,TickType_t time);
esp_err_t queue_receive(int dest,TickType_t time,msg_queue_t *msg);
esp_err_t queue_start(void);
int queue_load(void);
void queue_reset(void);
esp_err_t queue_receive_instat(int dest,msg_queue_t*msg);
#endif