#include "wifi.h"
#include "storage.h"
#include "event.h"
#include "esp_mac.h"


static int s_retry_num = 0;
extern EventGroupHandle_t Bits_events;
static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(__FUNCTION__, "retry to connect to the AP");
        } 
        else 
        {
            xEventGroupSetBits(Bits_events, WIFI_FAIL_BIT);
        }
        ESP_LOGE(__FUNCTION__,"connect to the AP fail");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(__FUNCTION__, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(Bits_events, WIFI_CONNECTED_BIT);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) 
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        ESP_LOGI(__FUNCTION__, "Station "MACSTR" joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) 
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        ESP_LOGI(__FUNCTION__, "Station "MACSTR" left, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

static esp_err_t set_dns_server(esp_netif_t *netif, uint32_t addr, esp_netif_dns_type_t type)
{
    if (addr && (addr != IPADDR_NONE)) {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = addr;
        dns.ip.type = IPADDR_TYPE_V4;
        ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, type, &dns));
    }
    return ESP_OK;

}
static esp_err_t static_ip(esp_netif_t *netif)
{
    size_t ip_len = storage_get_size("ip");
    esp_netif_ip_info_t Ip;
    memset(&Ip,0,sizeof(esp_netif_ip_info_t));
    if(ip_len >0)
    {
        if(esp_netif_dhcpc_stop(netif) != ESP_OK) 
        {
            ESP_LOGE(__FUNCTION__, "Error desactivado de dhcp");
            return ESP_FAIL;
        }
        else{ESP_LOGI(__FUNCTION__,"Dhcp parado con exito!");}


        char * ip = malloc(sizeof(char)*ip_len);
        if(ip == NULL)
        {
            ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
            return ESP_FAIL;
        }
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"ip",ip,&ip_len));
        Ip.ip.addr = ipaddr_addr((const char*)ip);
        free(ip);

        size_t netmask_len = storage_get_size("netmask");
        if(netmask_len >0)
        {
            char *netmask = malloc(sizeof(char) * netmask_len);
            if(netmask == NULL)
            {
                ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
                return ESP_FAIL;
            }
            ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"netmask",netmask,&netmask_len));
            Ip.netmask.addr = ipaddr_addr((const char*)netmask);
            free(netmask);
        }
        size_t gateway_len = storage_get_size("netmask");
        if(gateway_len >0)
        {
            char *gateway = malloc(sizeof(char) * gateway_len);
            if(gateway == NULL)
            {
                ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
                return ESP_FAIL;
            }
            ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"netmask",gateway,&gateway_len));
            Ip.gw.addr = ipaddr_addr((const char*)gateway);
            free(gateway);
        }
        if(esp_netif_set_ip_info(netif, &Ip) != ESP_OK)
        {
            ESP_LOGE(__FUNCTION__, "Fallo al cargar ip info");
            return ESP_FAIL;
        }
        else{ ESP_LOGI(__FUNCTION__,"Conf. ip cargada con exito!");} 
    }
    else
    {
        ESP_LOGI(__FUNCTION__,"[AUTO] DHCP automatico");
    }
    size_t dns1_len = storage_get_size("dns1");
    if(dns1_len > 0)
    {
        char *dns1 = malloc(sizeof(char) * dns1_len);
        if(dns1 == NULL)
        {
            ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
            return ESP_FAIL;
        }
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"dns1",dns1,&dns1_len));
        ESP_ERROR_CHECK(set_dns_server(netif, ipaddr_addr((const char*)dns1), ESP_NETIF_DNS_MAIN));
        free(dns1);
    }
    size_t dns2_len = storage_get_size("dns2");
    if(dns2_len > 0)
    {
        char *dns2 = malloc(sizeof(char) * dns2_len);
        if(dns2 == NULL)
        {
            ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
            return ESP_FAIL;
        }
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"dns2",dns2,&dns2_len));
        ESP_ERROR_CHECK(set_dns_server(netif, ipaddr_addr((const char*)dns2), ESP_NETIF_DNS_MAIN));
        free(dns2);
    }
    return ESP_OK;
}

esp_err_t wifi_init_sta(const char *ssid, const char* password)
{
    ESP_ERROR_CHECK(esp_netif_init());

    esp_netif_t *net = esp_netif_create_default_wifi_sta();
    static_ip(net);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

  
    wifi_config_t wifi_config = {0};

    strcpy((char *)wifi_config.sta.ssid,ssid);
    if(password != NULL)
    {
        strcpy((char *)wifi_config.sta.password, password);
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(__FUNCTION__, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(Bits_events,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(__FUNCTION__, "Conectado a SSID:%s",ssid);

        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(__FUNCTION__, "Fallo al conectar a SSID:%s",ssid);
        return ESP_FAIL;
    } else {
        ESP_LOGE(__FUNCTION__, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
    return ESP_FAIL;
}
char *wifi_scan(void)
{
    cJSON *json;
    cJSON *text;
    cJSON *msg;
    json = cJSON_CreateObject();
    msg = cJSON_CreateObject();
    uint16_t numero = 10;
    wifi_ap_record_t ap_lista[numero];
    uint16_t ap_count = 0;
    memset(ap_lista,0,sizeof(ap_lista));
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&numero, ap_lista));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
     for (int i = 0; (i < numero) && (i < ap_count); i++)
    { 
        text = cJSON_AddNumberToObject(json,(const char*)ap_lista[i].ssid,ap_lista[i].rssi);
        
    }
    cJSON_AddItemToObject(msg,"wifiscan",json);
    char *string = cJSON_Print(msg);
    cJSON_Delete(msg);
    return string;

}

esp_err_t wifi_Ap_call(void)
{

    Wifi_stop();
    ESP_ERROR_CHECK(wifi_init_softap());
    return ESP_OK;
}

esp_err_t wifi_init_softap(void)
{


    ESP_LOGI(__FUNCTION__,"ESP SOFT_AP");
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_t *ap = esp_netif_create_default_wifi_ap();
    assert(ap);
    esp_netif_t *_scan = esp_netif_create_default_wifi_sta();
    assert(_scan);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = SSID,
            .ssid_len = strlen(SSID),
            .channel = CANAL,
            .max_connection = MAXCON

        },
    };
    wifi_config_t wifi_sta = {0};
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(__FUNCTION__, "wifi_init_softap finished. SSID:%s  channel:%d",
             SSID,  CANAL);
    return ESP_OK;
}
void Wifi_stop(void)
{
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_netif_deinit();
    esp_wifi_set_mode(WIFI_MODE_NULL);

   
}

esp_err_t Wifi_start(void)
{
    ESP_ERROR_CHECK(Event_init());
    esp_err_t ret;
    nvs_flash_init();
    size_t ssid_len = storage_get_size("ssid");
    if( ssid_len > 0)
    {
        char *ssid = malloc(sizeof(char) * ssid_len);
        if(ssid == NULL)
        {
            ESP_LOGE(__FUNCTION__, "[284] sin memoria dinamica :(");
            return ESP_FAIL;
        }
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"ssid",ssid,ssid_len));
        size_t passwd_len = storage_get_size("password");
        if( passwd_len > 0)
        {
            char *passwd = malloc(sizeof(char) * ssid_len);
            if(passwd == NULL)
            {
                ESP_LOGE(__FUNCTION__, "[294] sin memoria dinamica :(");
                return ESP_FAIL;
            }
            ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"password",passwd,passwd_len));
            ret = wifi_init_sta(ssid,passwd);
            if(ret != ESP_OK )
            {
                wifi_Ap_call();
            }

        }
        else
        {
            ret = wifi_init_sta(ssid,NULL);
            if(ret != ESP_OK )
            {
                wifi_Ap_call();
            }
        }
    }
    else
    {
        ESP_LOGW(__FUNCTION__, "no ssid guardada en NVS");
    }
    

return ESP_OK;   
}


