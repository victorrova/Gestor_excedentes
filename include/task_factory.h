#ifndef TASK_FACTORY_H
#define TASK_FACTORY_H
#include "msgqueue.h"
#include "storage.h"
#include "event.h"
#include "machine.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "FreeRTOSConfig.h"
esp_err_t task_memory_control(const char *task_name);
esp_err_t task_create(TaskFunction_t task,const char * const name,UBaseType_t Priority,void *pvparams);
#endif