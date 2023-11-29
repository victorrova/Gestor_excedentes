#ifndef EVENT_H
#define EVENT_H

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define WIFI_MODE          BIT3
#define MQTT_CONNECT       BIT4
#define MQTT_DISCONNECT    BIT5
#define MQTT_ON_MESSAGE    BIT6


enum{                  // EVENTOS DE LA MAQUINA 
    MACHINE_OK,        // El arranque se ha completado con exito
    MACHINE_FAIL,      //  El arranque tiene un componente crito en fallo
    MACHINE_RESET,     // necesidada de reset
    MACHINE_SAVE,      // necesidad de guardadr cambios
    MACHINE_LOAD,      // necesidad de cargar cambios
    MACHINE_CONF_OK,   // configuración correcta
    MACHINE_CONF_FAIL, // configuración fallida
    QUEUE_OVERLOAD     // cola de mensajes llena
    
};

esp_err_t Event_init(void);
#endif