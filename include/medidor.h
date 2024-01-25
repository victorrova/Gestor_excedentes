#ifndef MEDIDOR_H
#define MEDIDOR_H

#include "string.h"
#include "helper.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "config.h"



#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

typedef struct {
    
    float Voltage;
    float Current;
    float Power_active;
    float Power_appa;
    float Pf;
} meter_t;

esp_err_t Hlw8032_Init(void);
esp_err_t Hlw8032_Read(meter_t* out);

#endif