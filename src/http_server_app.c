/*
 * http_server.c
 *
 */

#include "http_server_app.h"
/*#include "wifi_app.h"
#include "sntp_time_sync.h"
#include "app_nvs.h"*/

#include "FreeRTOS/event_groups.h"

// Tag used for ESP serial console messages
static const char TAG[] = "http_server";

// Wifi connect status
static int g_wifi_connect_status = NONE;

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

struct async_resp_arg
{
	httpd_handle_t hd;
	int fd;
	char *data;
};


extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const uint8_t app_css_start[] asm("_binary_app_css_start");
extern const uint8_t app_css_end[] asm("_binary_app_css_end");
extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");
extern const uint8_t favicon_ico_start[] asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[] asm("_binary_favicon_ico_end");

/**
 * HTTP server monitor task used to track events of the HTTP server
 * @param pvParameters parameter which can be passed to the task.
 */
static void http_server_monitor(void *parameter)
{
	http_server_queue_message_t msg;

	for (;;)
	{
		if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
			case HTTP_MSG_WIFI_CONNECT_INIT:
				ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_INIT");

				g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECTING;

				break;

			case HTTP_MSG_WIFI_CONNECT_SUCCESS:
				ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_SUCCESS");

				g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_SUCCESS;
				//app_nvs_load_mqtt_creds();

				break;

			case HTTP_MSG_WIFI_CONNECT_FAIL:
				ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_FAIL");

				g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_FAILED;

				break;

			case HTTP_MSG_WIFI_USER_DISCONNECT:
				ESP_LOGI(TAG, "HTTP_MSG_WIFI_USER_DISCONNECT");

				g_wifi_connect_status = HTTP_WIFI_STATUS_DISCONNECTED;

				break;

			default:
				break;
			}
		}
	}
}

/**
 * Jquery get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
/* static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Jquery requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

	return ESP_OK;
}
 */
/**
 * Sends the index.html page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "index.html requested");

	httpd_resp_set_status(req, "307 Temporary Redirect");
	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

	return ESP_OK;
}

/**
 * app.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "app.css requested");

	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)app_css_start, app_css_end - app_css_start);

	return ESP_OK;
}

/**
 * app.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "app.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);

	return ESP_OK;
}

/**
 * Sends the .ico (icon) file when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "favicon.ico requested");

	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

	return ESP_OK;
}

/**
 * wifiConnectStatus handler updates the connection status for the web page.
 */
cJSON *WS_wifi_connect_status(void)
{
	ESP_LOGI(TAG, "WS wifiConnectStatus requested");

	cJSON *root, *wifi_status;

	root = cJSON_CreateObject();
	wifi_status = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "id", cJSON_CreateString("WIFISTATUS"));
	cJSON_AddItemToObject(wifi_status, "STATUS", cJSON_CreateNumber(g_wifi_connect_status));
	cJSON_AddItemToObject(root, "data", wifi_status);

	return root;
}

/**
 * wifiConnectInfo.json handler updates the connection info for the web page.
 */
cJSON *WS_wifi_connect_info(void)
{
	ESP_LOGI(TAG, "WS wifiConnectInfo requested");

	cJSON *root, *wifi_info;

	char ip[IP4ADDR_STRLEN_MAX];
	char netmask[IP4ADDR_STRLEN_MAX];
	char gw[IP4ADDR_STRLEN_MAX];

	root = cJSON_CreateObject();
	wifi_info = cJSON_CreateObject();

	cJSON_AddItemToObject(root, "id", cJSON_CreateString("WIFIINFO"));

	/*if (g_wifi_connect_status == HTTP_WIFI_STATUS_CONNECT_SUCCESS)
	{
		wifi_ap_record_t wifi_data;
		ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&wifi_data));
		char *ssid = (char *)wifi_data.ssid;

		esp_netif_ip_info_t ip_info;
		ESP_ERROR_CHECK(esp_netif_get_ip_info(esp_netif_sta, &ip_info));
		esp_ip4addr_ntoa(&ip_info.ip, ip, IP4ADDR_STRLEN_MAX);
		esp_ip4addr_ntoa(&ip_info.netmask, netmask, IP4ADDR_STRLEN_MAX);
		esp_ip4addr_ntoa(&ip_info.gw, gw, IP4ADDR_STRLEN_MAX);

		cJSON_AddItemToObject(wifi_info, "ssid", cJSON_CreateString(ssid));
		cJSON_AddItemToObject(wifi_info, "ip", cJSON_CreateString(ip));
		cJSON_AddItemToObject(wifi_info, "netmask", cJSON_CreateString(netmask));
		cJSON_AddItemToObject(wifi_info, "gw", cJSON_CreateString(gw));
		cJSON_AddItemToObject(root, "data", wifi_info);
	}*/

	return root;
}

// Función para la gestión de la conexión/desconexión de la comunicación MQTT por mensaje WS recibido
void ws_mqtt_app(cJSON *root)
{
	ESP_LOGI(TAG, "WS MQTT");

	cJSON *data, *connect, *broker, *user, *pass;

	data = cJSON_GetObjectItem(root, "data");
	connect = cJSON_GetObjectItem(data, "connect");

	// Manda la orden de conectar con el broker
	if (connect->valueint == 1)
	{

		ESP_LOGI(TAG, "MQTT Connect requested");

		broker = cJSON_GetObjectItem(data, "broker");
		user = cJSON_GetObjectItem(data, "user");
		pass = cJSON_GetObjectItem(data, "pass");
		/*
		esp_mqtt_client_config_t mqtt_cfg = {
			.broker.address.transport = MQTT_TRANSPORT_OVER_TCP,
			.broker.address.hostname = broker->valuestring,
			.credentials.username = user->valuestring,
			.credentials.client_id = "SEC",
			.credentials.authentication.password = pass->valuestring,
			.broker.address.port = 1883,
		};

		mqtt_app_start(mqtt_cfg);*/
	}

	else
	{
		ESP_LOGI(TAG, "MQTT Disconect requested");
		//mqtt_app_disconnect();
	}
}

// Función para la gestión de la conexión/desconexión del wifi por mensaje WS recibido
char *ws_wifi_app(cJSON *root)
{
	ESP_LOGI(TAG, "WS WIFI");

	cJSON *data, *connect, *ssid, *pass;
	char *send_data_json_string = (char *)malloc(sizeof(char));

	data = cJSON_GetObjectItem(root, "data");
	connect = cJSON_GetObjectItem(data, "connect");

	if (connect->valueint == DISCONNECT_WIFI_FROM_STA) // Wifi Disconnect
	{
		ESP_LOGI(TAG, "wifi Disconect requested");

		//wifi_app_send_message(WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT);
	}

	else if (connect->valueint == CONNECT_WIFI_FROM_STA) // Wifi Connect: Manda la orden de conectar con el SSID y el Pass
	{

		ESP_LOGI(TAG, "wifi Connect requested");

		ssid = cJSON_GetObjectItem(data, "ssid");
		pass = cJSON_GetObjectItem(data, "pass");

		char *ssid_str = NULL, *pass_str = NULL;

		ssid_str = ssid->valuestring;
		pass_str = pass->valuestring;

		/*if (wifi_app_creds_sta(ssid_str, pass_str) == ESP_OK)
		{
			ESP_LOGI(TAG, "SSID & Pass send!");
		}*/

		free(ssid_str);
		free(pass_str);
	}

	/*else if (connect->valueint == SCAN_WIFI_STA) // Wifi Scan list
	{
		// char *my_json_string;
		// my_json_string = (char *)malloc(sizeof(char));
		ESP_LOGI(TAG, "wifi scan requested");

		cJSON *send_data = wifi_app_scan();

		send_data_json_string = cJSON_PrintUnformatted(send_data);

		ESP_LOGI(TAG, "send_data_json_string\n%s", send_data_json_string);

		cJSON_Delete(send_data);
	}

	else if (connect->valueint == STATUS_WIFI_STA)
	{
		// Devuelve wifi status al cliente
		cJSON *send_data = WS_wifi_connect_status();
		send_data_json_string = cJSON_PrintUnformatted(send_data);
		cJSON_Delete(send_data);
	}

	else if (connect->valueint == INFO_WIFI_STA)
	{
		// Devuelve wifi info al cliente
		cJSON *send_data = WS_wifi_connect_info();
		send_data_json_string = cJSON_PrintUnformatted(send_data);
		cJSON_Delete(send_data);
		ESP_LOGI(TAG, "WS wifiConnectInfo requested: %s", send_data_json_string);
	}*/

	return send_data_json_string;
}

/*
 * async send function, which we put into the httpd work queue
 */
static void ws_async_send(void *arg)
{

	struct async_resp_arg *resp_arg = arg;
	char *data = resp_arg->data;
	httpd_handle_t hd = resp_arg->hd;
	int fd = resp_arg->fd;
	httpd_ws_frame_t ws_pkt;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.payload = (uint8_t *)data;
	ws_pkt.len = strlen(data);
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;

	httpd_ws_send_frame_async(hd, fd, &ws_pkt);
	free(resp_arg);
}

static esp_err_t trigger_async_send(httpd_handle_t handle, httpd_req_t *req, char *data)
{
	struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
	resp_arg->hd = handle;
	resp_arg->fd = httpd_req_to_sockfd(req);
	resp_arg->data = data;
	return httpd_queue_work(handle, ws_async_send, resp_arg);
}

// Función que recoge los WS enviados desde el cliente
static esp_err_t handle_ws_req(httpd_req_t *req)
{
	if (req->method == HTTP_GET)
	{
		ESP_LOGI(TAG, "Handshake done, the new connection was opened");
		return ESP_OK;
	}

	httpd_ws_frame_t ws_pkt;
	uint8_t *buf = NULL;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
		return ret;
	}

	if (ws_pkt.len)
	{
		buf = calloc(1, ws_pkt.len + 1);
		if (buf == NULL)
		{
			ESP_LOGE(TAG, "Failed to calloc memory for buf");
			return ESP_ERR_NO_MEM;
		}
		ws_pkt.payload = buf;
		ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
		if (ret != ESP_OK)
		{
			ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
			free(buf);
			return ret;
		}
		ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
	}

	ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);

	if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) // Aquí se filtran los mensaje WS y se envían a la función que les corresponda
	{
		char *data = (char *)malloc(sizeof(char));

		cJSON *id;
		cJSON *root = cJSON_Parse((char *)ws_pkt.payload);
		id = cJSON_GetObjectItem(root, "id");
		ESP_LOGI(TAG, "id: %s", id->valuestring);

		// Filtro de ID para direccionar los datos
		if (strcmp(id->valuestring, "MQTT") == 0)
		{
			ws_mqtt_app(root);
			trigger_async_send(req->handle, req, (char *)data); // devuelve un mensaje WS al clinete
		}

		else if (strcmp(id->valuestring, "WIFI") == 0)
		{
			data = ws_wifi_app(root);
			trigger_async_send(req->handle, req, (char *)data); // devuelve un mensaje WS al clinete
		}

		else if (strcmp(id->valuestring, "TIME") == 0)
		{
			// Función para devolver fecha y hora al cliente

			char *timestamp_json_string;
			timestamp_json_string = (char *)malloc(sizeof(char));

			/*if (sntp_json_time(&timestamp_json_string) == ESP_OK)
			{

				trigger_async_send(req->handle, req, timestamp_json_string);
				ESP_LOGI(TAG, "Enviado time: %s", timestamp_json_string);
			}*/
		}

		else
		{
			ESP_LOGI(TAG, "Id no conocida");
		}

		free(buf);
	}

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////////////

// HTTP Error (404) Handler - Redirects all requests to the root page
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
	// Set status
	httpd_resp_set_status(req, "302 Temporary Redirect");
	// Redirect to the "/" root directory
	httpd_resp_set_hdr(req, "Location", "/");
	// iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
	httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

	ESP_LOGI(TAG, "Redirecting to root");
	return ESP_OK;
}

/**
 * Sets up the default httpd server configuration.
 * @return http server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
	// Generate the default configuration
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Create HTTP server monitor task
	xTaskCreatePinnedToCore(&http_server_monitor, "http_server_monitor", HTTP_SERVER_MONITOR_STACK_SIZE, NULL, HTTP_SERVER_MONITOR_PRIORITY, &task_http_server_monitor, HTTP_SERVER_MONITOR_CORE_ID);

	// Create the message queue
	http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));

	// The core that the HTTP server will run on
	config.core_id = HTTP_SERVER_TASK_CORE_ID;

	// Adjust the default priority to 1 less than the wifi application task
	config.task_priority = HTTP_SERVER_TASK_PRIORITY;

	// Bump up the stack size (default is 4096)
	config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

	// Increase uri handlers
	config.max_uri_handlers = 20;

	// Increase the timeout limits
	config.recv_wait_timeout = 10;
	config.send_wait_timeout = 10;

	ESP_LOGI(TAG,
			 "http_server_configure: Starting server on port: '%d' with task priority: '%d'",
			 config.server_port,
			 config.task_priority);

	// Start the httpd server
	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{
		ESP_LOGI(TAG, "http_server_configure: Registering URI handlers");

		// register index.html handler
		httpd_uri_t index_html = {
			.uri = "/",
			.method = HTTP_GET,
			.handler = http_server_index_html_handler,
			.user_ctx = NULL};
		httpd_register_uri_handler(http_server_handle, &index_html);

		// register app.css handler
		httpd_uri_t app_css = {
			.uri = "/app.css",
			.method = HTTP_GET,
			.handler = http_server_app_css_handler,
			.user_ctx = NULL};
		httpd_register_uri_handler(http_server_handle, &app_css);

		// register app.js handler
		httpd_uri_t app_js = {
			.uri = "/app.js",
			.method = HTTP_GET,
			.handler = http_server_app_js_handler,
			.user_ctx = NULL};
		httpd_register_uri_handler(http_server_handle, &app_js);

		// register favicon.ico handler
		httpd_uri_t favicon_ico = {
			.uri = "/favicon.ico",
			.method = HTTP_GET,
			.handler = http_server_favicon_ico_handler,
			.user_ctx = NULL};
		httpd_register_uri_handler(http_server_handle, &favicon_ico);

		// register Websocket handler
		httpd_uri_t ws = {
			.uri = "/ws",
			.method = HTTP_GET,
			.handler = handle_ws_req,
			.user_ctx = NULL,
			.is_websocket = true};

		httpd_register_uri_handler(http_server_handle, &ws);

		httpd_register_err_handler(http_server_handle, HTTPD_404_NOT_FOUND, http_404_error_handler);

		return http_server_handle;
	}

	return NULL;
}

void http_server_start(void)
{
	if (http_server_handle == NULL)
	{
		http_server_handle = http_server_configure();
	}
}

void http_server_stop(void)
{
	if (http_server_handle)
	{
		httpd_stop(http_server_handle);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server");
		http_server_handle = NULL;
	}
	if (task_http_server_monitor)
	{
		vTaskDelete(task_http_server_monitor);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server monitor");
		task_http_server_monitor = NULL;
	}
}

BaseType_t http_server_monitor_send_message(http_server_message_e msgID)
{
	http_server_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(http_server_monitor_queue_handle, &msg, portMAX_DELAY);
}
