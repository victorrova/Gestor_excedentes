

#include "task_factory.h"



ESP_EVENT_DECLARE_BASE(MACHINE_EVENTS);

esp_err_t task_create(TaskFunction_t task,const char *name,UBaseType_t Priority,void *pvparams)
{
    uint32_t stack = 0;
    
    uint8_t count = 0;
    size_t Ustack = storage_get_size(name);
    if(Ustack>0)
    {
        printf("entra para estack\n");
        storage_load(NVS_TYPE_U32,name,stack,NULL);
        printf("entra para estack salida %lu\n",stack);
    }
    else
    {
        stack = INIT_STACK;

    }
    BaseType_t result;

    do
    {
        result = xTaskCreate(task,name,stack,pvparams,Priority,NULL);
        vTaskDelay(50/portTICK_PERIOD_MS);
        count ++;
        if(count > 100)
        {
            ESP_LOGE(__FUNCTION__,"Imposible crear tarea");
            return ESP_FAIL;
        }
    } while (result != pdPASS);

    esp_event_post(MACHINE_EVENTS,MACHINE_TASK_CALLL,name,strlen(name),portMAX_DELAY);
    ESP_LOGI(__FUNCTION__,"tarea: %s creada con éxito pila = %lu",name,stack);
    return ESP_OK;
}

esp_err_t task_memory_control(const char *task_name)
{
    TaskHandle_t task_Handle = NULL;
    TaskStatus_t status;
    task_Handle = xTaskGetHandle(task_name);
    esp_err_t err = ESP_FAIL;
    if(task_Handle == NULL)
    {
        return err;
    }
    vTaskGetInfo(task_Handle,&status,pdTRUE,eInvalid);
    uint32_t stack = (INIT_STACK  - status.usStackHighWaterMark) * 1.30;
    size_t Ustack = storage_get_size(task_name);
    if(Ustack > 0)
    {
        storage_erase_key(task_name);
    }
    err = storage_save(NVS_TYPE_U32,task_name,(uint32_t)stack);
    ESP_LOGW(__FUNCTION__,"Pila de memoria de %s = %lu",task_name,stack);
    return err;
}