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
}logger_t;
/*inicio del termistor NTC*/
esp_err_t termistor_init(void);
/*inicio de la salida del ventilador y prueba su funcionamiento */
esp_err_t Fan_init(void);
/*cambia el estado del ventilador*/
void Fan_state(int state);
/*lee y traduce a grados Cº la lectura del NTC*/
float temp_termistor(void);
/*cambia el canal de logger*/
void set_stream_logger(int logger);
/* Inicio del Meter  Hlw8032*/
esp_err_t Meter_init(void);
/*función que manda  mensaje mqtt con el estado del dimmer  */
esp_err_t Keepalive(int state_gestor, char *exit);
/*llamada al Ap de configuración*/
esp_err_t Ap_call_Init(void);
/*devuelve la memoria libre en bytes*/
uint32_t free_mem(void);
int Fan_get_state(void);
#endif