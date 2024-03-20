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
/*
Funciones creadas para poder controlar la ram que usa cada tarea,
almacenarla en nvs y a la hora de crear la tarea dinámicamente,
recuperar la ram necesraia más un tanto por ciento de incremento
(configurable en config.h)
*/
/*función que recupera y guarda en nvs la ram necesaria por nombre de la tarea*/
esp_err_t task_memory_control(char *task_name);
/*creador de tareas Freertos, que adjudica la ram necesaria cada vez que se crea*/

esp_err_t task_create(TaskFunction_t task, const char *const name, UBaseType_t Priority, void *pvparams);
void stop_task(char *task_name);
#endif