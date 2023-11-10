#ifndef DIMMER_H
#define DIMMMER_H
#include <stdio.h>
#include <string.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "pid.h"
#include "msgqueue.h"
#include "cJSON.h"
#include "Kostal.h"
#include "machine.h"
#include "helper.h"
#include "storage.h"

typedef struct conf_dimmmer{
    char inverter_url[128];
    int min_delay;                          // tiempo minimo de apertura de Triac
    int result;                             // tiempo maximo de apertura de Triac                           
    int Ext_Temp;                           // temperatura sonda externa
    int level;                              // Porcentaje nivel de apertura de Triac
    int reg;                                // Nivel de regulacion nivel en tanto por ciento
    bool _enable;                           // control de on-off
    bool eco_mode;                          // activacion modo eco
    bool enable_Ext_Temp;                   // activacion control externo de temperatura
    esp_timer_handle_t _timer;   
    PID_IncTypeDef pid_Pwr;
    PID_IncTypeDef pid_NTC;
}conf_dimmer_t;


void dimmer_init(void);

#endif