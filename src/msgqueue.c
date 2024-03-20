

#include "msgqueue.h"


static QueueHandle_t msg_queue;

esp_err_t queue_send(int dest,const char* payload, const char* topic,TickType_t time)
{
    if(strlen(payload)>MAX_PAYLOAD || strlen(topic) > MAX_TOPIC)
    {
        ESP_LOGE(__FUNCTION__,"payload [%d] o topic [%d] demasiado largo",(int)strlen(payload),(int)strlen(topic));
        return ESP_FAIL;
    }
    msg_queue_t *msg = (msg_queue_t*)pvPortMalloc(sizeof(msg_queue_t));
    ESP_MALLOC_CHECK(msg);
    msg->dest = dest;
    strcpy(msg->msg,payload);
    msg->len_msg = strlen(payload);
    if(topic != NULL)
    {
        strcpy(msg->topic,topic);
        msg->len_topic = strlen(topic);
    }
    msg->count =0;
    xQueueSend(msg_queue,msg,time);
    vPortFree(msg);
    return ESP_OK;

}

esp_err_t queue_receive(int dest,TickType_t time,msg_queue_t *msg)
{   
    if(xQueueReceive(msg_queue,msg,time) == pdTRUE)
    {
        if(dest == MASTER || msg->dest == dest)
        {
            ESP_LOGD(__FUNCTION__,"mensaje entregado");
            return ESP_OK;
        }
        else if(msg->dest != dest  && msg->count < QUEUE_MAX_LAP )
        {
            msg->count++;
            xQueueSend(msg_queue,msg,time);
            ESP_LOGD(__FUNCTION__,"mensaje devuelto");
            msg->len_msg = 0;
            return ESP_FAIL;
        }
        else if(msg->dest != dest && msg->count >= QUEUE_MAX_LAP)
        {
            msg->len_msg = 0;
            ESP_LOGE(__FUNCTION__,"mensaje huerfano eliminado en lap =  %d",msg->count);
            return ESP_FAIL;
        }
    }
    msg->len_msg= 0;
    return ESP_FAIL;
      
}
esp_err_t queue_receive_instat(int dest,msg_queue_t *msg)
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
