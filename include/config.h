#ifndef  CONFIG_H
#define  CONFIG_H




/*wifi*/
#define EXAMPLE_ESP_MAXIMUM_RETRY             10
#define SSID                                  "Gestor"
#define CANAL                                 1
#define PASS 
#define MAXCON                                1

/*pinout*/
#define FAN                                   19
#define SELECT                                0 
#define ZERO                                  18 
#define TRIAC                                 14

/*ota*/
#define OTA_URL_SIZE                           256
/* VERSION*/
#define VERSION                                0.2
/* meter*/
#define METER_ENABLE                           1
#define UART_PORT                              2
#define RX_PIN                                 16
#define BUFFER                                 256
#define VOLTAGE_COEF                           1.88
#define CURRENT_COEF                           1
#define DEBUG                                  0
#define POWER_CHARGER                          1500
#define CF                                     1
/*cola  mensajes*/
#define QUEUE_MAX_LAP                          5
#define QUEUE_SIZE                             5
#define MAX_PAYLOAD                            512
#define MAX_TOPIC                              128


/*task factory*/
#define INIT_STACK                             10000
#define INC_VALUE                              1.30

// HTTP Server task
#define HTTP_SERVER_TASK_STACK_SIZE				8192
#define HTTP_SERVER_TASK_PRIORITY				4
#define HTTP_SERVER_TASK_CORE_ID				0

/*dimmer*/
#define KEEPALIVE_LAP                           900
#define MAX_POWER                               3600
#define LIMIT_TEMP                              35
#endif