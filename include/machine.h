#ifndef MACHINE_H
#define MACHINE_H


#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
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
#include "wifi.h"
#include "storage.h"
#include "cJSON.h"
#include "medidor.h"
#include "config.h"

typedef enum{
    WS_LOGGER,
    MQTT_LOGGER,
    OLED_LOGGER,
    SERIAL_LOGGER
};

esp_err_t termistor_init(void);
esp_err_t Fan_init(void);
void Fan_state(int state);
float temp_termistor(void);
void set_stream_logger(int logger);
esp_err_t Meter_init(void);
esp_err_t Keepalive(int state_gestor, char *exit);
esp_err_t Ap_call_Init(void);
uint32_t free_mem(void);
void Keepalive_task(void *params);
#endif