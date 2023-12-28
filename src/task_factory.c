

#include "task_factory.h"



esp_err_t task_create(TaskFunction_t task,const char * const name,UBaseType_t Priority,void *pvparams)
{
    uint32_t stack;
    if(storage_get_size(name)>0)
    {
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,name,stack,NULL));
    }
    else
    {
        stack = 20000;
    }
}