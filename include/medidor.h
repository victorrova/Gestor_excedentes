#ifndef MEDIDOR_H
#define MADIDOR_H

#include "string.h"
#include "helper.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define UART_PORT 2
#define RX_PIN 16
#define BUFFER 256
#define VOLTAGE_COEF 1.88
#define CURRENT_COEF 1
#define DEBUG 1

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

typedef struct{
    float Voltage;
    float Current;
    float Power_active;
    float Power_appa;
    float Pf;

}meter_t;
esp_err_t Hlw8032_Init(void);
esp_err_t Hlw8032_read(meter_t *meter);
#endif