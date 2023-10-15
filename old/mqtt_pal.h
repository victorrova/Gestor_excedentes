#ifndef MQTT_PAL_H
#define MQTT_PAL_H

#include <stdio.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <esp_timer.h>
#include <errno.h>
#include <netdb.h>            // struct addrinfo
#include <arpa/inet.h>
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_err.h"

typedef time_t mqtt_pal_time_t;
typedef SemaphoreHandle_t mqtt_pal_mutex_t;
typedef int mqtt_pal_socket_handle;
#define MQTT_PAL_HTONS(s) htons(s)
#define MQTT_PAL_NTOHS(s) ntohs(s)




float esp_time_s(void);

void _mutex_init(mqtt_pal_mutex_t xema);

int open_socket(const char* addr, const char* port);
int tcp_client(const char* addr, int port);
ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, ssize_t len, int flags);


ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, ssize_t bufsz, int flags);



#endif
