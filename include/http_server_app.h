/*
 * http_server.h
 *
 */


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#include <esp_system.h>

#include <sys/param.h>

#include "esp_netif.h"
#include "esp_vfs.h"

#include "esp_tls_crypto.h"
#include <esp_http_server.h>
//#include "dns_server_app.h"

#include "tasks_common.h"
#include "cJSON.h"

#ifndef MAIN_HTTP_SERVER_H_
#define MAIN_HTTP_SERVER_H_


/**
 * Connection status for Wifi
 */
typedef enum http_server_wifi_connect_status
{
	NONE = 0,
	HTTP_WIFI_STATUS_CONNECTING,
	HTTP_WIFI_STATUS_CONNECT_FAILED,
	HTTP_WIFI_STATUS_CONNECT_SUCCESS,
	HTTP_WIFI_STATUS_DISCONNECTED,
} http_server_wifi_connect_status_e;

/**
 * Messages for the HTTP monitor
 */
typedef enum http_server_message
{
	HTTP_MSG_WIFI_CONNECT_INIT = 0,
	HTTP_MSG_WIFI_CONNECT_SUCCESS,
	HTTP_MSG_WIFI_CONNECT_FAIL,
	HTTP_MSG_WIFI_USER_DISCONNECT,
	HTTP_MSG_OTA_UPDATE_SUCCESSFUL,
	HTTP_MSG_OTA_UPDATE_FAILED,
	HTTP_MSG_TIME_SERVICE_INITIALIZED,
} http_server_message_e;

/**
 * Client actions for Wifi
 */
typedef enum client_actions_wifi
{
	DISCONNECT_WIFI_FROM_STA = 0,
	CONNECT_WIFI_FROM_STA,
	SCAN_WIFI_STA,
	STATUS_WIFI_STA,
	INFO_WIFI_STA,
} client_actions_wifi_e;

/**
 * Client actions for MQTT
 */
typedef enum client_actions_mqtt
{
	DISCONNECT_MQTT_FROM_BROKER = 0,
	CONNECT_MQTT_FROM_BROKER,
	STATUS_MQTT,
} client_actions_mqtt_e;

/**
 * PID actions
 */
typedef enum client_actions_pid
{
	STREAM_PID_VALUE = 0,
	STORAGE_PID_VALUE,
	
} client_actions_pid_e;

/**
 * Inverter actions
 */
typedef enum client_actions_inverter
{
	STREAM_INVERTER_VALUE = 0,
	STORAGE_INVERTER_VALUE,
	
} client_actions_inverter_e;

/**
 * Structure for the message queue
 */
typedef struct http_server_queue_message
{
	http_server_message_e msgID;
} http_server_queue_message_t;


/**
 * Starts the HTTP server.
 */
void http_server_start(void);

/**
 * Stops the HTTP server.
 */
void http_server_stop(void);

/**
 * Timer callback function which calls esp_restart upon successful firmware update.
 */
void http_server_fw_update_reset_callback(void *arg);



#endif /* MAIN_HTTP_SERVER_H_ */
