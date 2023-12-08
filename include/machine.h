#ifndef MACHINE_H
#define MACHINE_H


#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include "soc/efuse_reg.h"
#include "esp_efuse.h"
#include <freertos/task.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "math.h"
#include "msgqueue.h"
#include "cJSON.h"


typedef struct s_timer{
 int prescaler;
 int timer;
 int(*function_cb)(void);
 int count;
 void *params;
 int result;
} s_timer_t;


int _free_mem(void);
esp_err_t termistor_init(void);
esp_err_t Fan_init(void);
void Fan_state(int state);
float temp_termistor(void);
void timer_init(s_timer_t *param,int prescaler,int timer,int (*callback)(void), void *params);
void timer_loop(s_timer_t *param);
void set_stream_logger(int logger);



#endif