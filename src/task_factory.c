

#include "task_factory.h"


extern EventGroupHandle_t Bits_events;


esp_err_t task_create(TaskFunction_t task,const char * const name,UBaseType_t Priority,void *pvparams)
{
    uint32_t stack = 0;
    esp_err_t err = ESP_FAIL;
    uint8_t count = 0;
    if(storage_get_size(name)>0)
    {
        err = storage_load(NVS_TYPE_U32,name,stack,NULL);
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
            return ESP_FAIL;
        }
    } while (result != pdPASS);
    xEventGroupSetBits(Bits_events,TASK_CALL);
    return err;
}

void task_memmory_control(void)
{

}