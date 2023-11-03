
#include <stdio.h>
#include <string.h>
#include "msgqueue.h"
#include "esp_log.h"
#include "esp_err.h"

QueueHandle_t msg_queue;

esp_err_t queue_send(int dest,const char* payload, const char* topic,TickType_t time)
{
    if(strlen(payload)>512 || strlen(topic) > 128)
    {
        ESP_LOGE(__FUNCTION__,"payload [%d] o topic [%d] demasiado largo",(int)strlen(payload),(int)strlen(topic));
        return ESP_FAIL;
    }
     msg_queue_t msg;
     msg.dest = dest;
     strcpy(msg.msg,payload);
     msg.len_msg = strlen(payload);
     if(topic != NULL)
    {
        strcpy(msg.topic,topic);
        msg.len_topic = strlen(topic);
    }
     msg.count = 0;
     xQueueSend(msg_queue,&msg,time);
     return ESP_OK;

}

msg_queue_t queue_receive(int dest,TickType_t time)
{   
    msg_queue_t msg;
    if(xQueueReceive(msg_queue,&msg,time) == pdTRUE)
    {
        if(msg.dest == dest)
        {
            ESP_LOGD(__FUNCTION__,"mensaje entregado");
            return msg;
        }
        else if(msg.dest != dest && msg.count < QUEUE_MAX_LAP)
        {
            msg.count++;
            xQueueSend(msg_queue,&msg,time);
            ESP_LOGD(__FUNCTION__,"mensaje devuelto");
            msg.len_msg = 0;
            return msg;
        }
        else if(msg.dest != dest && msg.count >= QUEUE_MAX_LAP)
        {
            msg.len_msg = 0;
            ESP_LOGD(__FUNCTION__,"mensaje huerfano eliminado en lap =  %d",msg.count);
            return msg;
        }

    }
    msg.len_msg= 0;
    return msg;
      
}
msg_queue_t Master_queue_receive(TickType_t time)
{   msg_queue_t msg;
    if(xQueueReceive(msg_queue,&msg,time) != pdTRUE)
    {
        msg.len_msg = 0;
    }
    return msg;
}

esp_err_t queue_start(void)
{
    msg_queue = xQueueCreate(QUEUE_SIZE, sizeof(msg_queue_t));
    if(msg_queue == NULL)
    {
        ESP_LOGE(__FUNCTION__,"no se ha podido crear la cola");
        return ESP_FAIL;  
    }
    return ESP_OK;
}


int queue_load(void)
{   
    return uxQueueMessagesWaiting(msg_queue);
}


void queue_reset(void)
{
    xQueueReset(msg_queue);
}
