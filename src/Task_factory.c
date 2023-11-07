
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// CONCEPTO
static TaskStatus_t *task_list;

typedef struct mem_task{
    TaskHandle_t task_handle;
    const char *name;
    uint32_t stack;
    void  (*function)(void);
    void *parameters;
    int core;
}mem_task_t;


void create_task()
{
 
}