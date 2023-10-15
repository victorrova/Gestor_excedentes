

#include "mqtt.h"

void _mutex_init(mqtt_pal_mutex_t xema)
{
    if(xema == NULL)
    {
        xema = xSemaphoreCreateMutex();
    }
}

float esp_time_s(void)
{
    return (int)esp_timer_get_time() / 1000000;
}

int open_socket(const char* addr, const char* port)
{
    struct addrinfo hints = {
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
    };
    int sockfd = -1;
    int rv;
    struct addrinfo *p, *servinfo;
    ESP_LOGI(__FUNCTION__,"aqui llegamos0");
    rv = getaddrinfo(addr, port, &hints, &servinfo);
    ESP_LOGI(__FUNCTION__,"aqui llegamos");
    if(rv != 0)
    {
        ESP_LOGE(__FUNCTION__,"getaddrinfo erro:%s",strerror(rv));
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(__FUNCTION__,"get info ok");
    }
    for(p = servinfo; p != NULL; p = p->ai_next) 
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        ESP_LOGI(__FUNCTION__,"aqui llegamos2");
        if (sockfd == -1)
        {
            continue;
        }
        rv = connect(sockfd, p->ai_addr, p->ai_addrlen);
        ESP_LOGI(__FUNCTION__,"aqui llegamos3");
        if(rv == -1) {
          close(sockfd);
          sockfd = -1;
          continue;
        }
        break;
    }
    freeaddrinfo(servinfo);
    return sockfd;
}
int tcp_client(const char* addr, int port)
{
  
    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    inet_pton(AF_UNSPEC, addr, &dest_addr.sin_addr);
    dest_addr.sin_family = AF_UNSPEC;
    dest_addr.sin_port = htons(port);

    addr_family = AF_UNSPEC;
    ip_protocol = IPPROTO_IP;
    int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(__FUNCTION__, "Unable to create socket: errno %d", errno);
        return -1;
    }
    ESP_LOGI(__FUNCTION__, "Socket created, connecting to %s:%d", addr, port);

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(__FUNCTION__, "Socket unable to connect: errno %d", errno);
        return -1;
    }
    ESP_LOGI(__FUNCTION__, "Successfully connected");
    return sock;

}

ssize_t mqtt_pal_sendall(mqtt_pal_socket_handle fd, const void* buf, ssize_t len, int flags) {
    ssize_t sent = 0;
    while(sent < len) {
        ssize_t tmp = send(fd, (char*)buf + sent, len - sent, flags);
        if (tmp < 0) {
            return MQTT_ERROR_SOCKET_ERROR;
        }
        sent += (ssize_t) tmp;
    }
    return sent;
}
ssize_t mqtt_pal_recvall(mqtt_pal_socket_handle fd, void* buf, ssize_t bufsz, int flags) {
    const char *const start = buf;
    ssize_t rv;
    do {
        rv = recv(fd, buf, bufsz, flags);
        if (rv > 0) {
            buf = (char*)buf + rv;
            bufsz -= rv;
        } else if (rv < 0) {
            return MQTT_ERROR_SOCKET_ERROR;
        }
    } while (rv > 0 && bufsz > 0);

    return (ssize_t)((char*)buf - start);
}
