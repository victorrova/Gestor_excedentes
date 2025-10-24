

#include "msgqueue.h"


static QueueHandle_t msg_queue;

esp_err_t queue_send(msg_destination dest,void* payload, msg_type type,TickType_t time)
{
    msg_queue_t *msg = (msg_queue_t*)malloc(sizeof(msg_queue_t));
    msg->dest = dest;
    msg->type = type;
    msg->count =0;
    msg->payload = payload;
    xQueueSend(msg_queue,msg,time);
    free(msg);
    return ESP_OK;

}

esp_err_t queue_receive(msg_destination dest,TickType_t time,msg_queue_t *msg)
{   
    if(xQueueReceive(msg_queue,msg,time) == pdTRUE)
    {
        if(dest == MASTER || msg->dest == dest)
        {
            ESP_LOGW(__FUNCTION__,"mensaje entregado");
            return ESP_OK;
        }
        else if(msg->dest != dest  && msg->count < QUEUE_MAX_LAP )
        {
            msg->count++;
            xQueueSend(msg_queue,msg,time);
            ESP_LOGW(__FUNCTION__,"mensaje devuelto");
            return ESP_FAIL;
        }
        else if(msg->dest != dest && msg->count >= QUEUE_MAX_LAP)
        {
            
            ESP_LOGE(__FUNCTION__,"mensaje huerfano eliminado en lap =  %d",msg->count);
            return ESP_FAIL;
        }
    }
    return ESP_FAIL;
      
}
esp_err_t queue_receive_instat(msg_destination dest,msg_queue_t *msg)
{
    int load = queue_load();
    while(xQueueReceive(msg_queue,msg,portMAX_DELAY) == pdTRUE)
    {
        if(msg->dest == dest) 
        {
            return ESP_OK;
        }
        else if(load == 0)
        {
            return ESP_FAIL;
        }
        else
        {
            msg->count++;
            xQueueSend(msg_queue,&msg,portMAX_DELAY);
        }
        load --;
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
    return ESP_FAIL;
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
