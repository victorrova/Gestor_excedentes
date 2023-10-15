#include "wifi.h"
#include <storage.h>
#include "event.h"

static EventGroupHandle_t s_wifi_event_group;



static int s_retry_num = 0;
static bool scan = false;

static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)

{
    static int ok = 0;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGD(__FUNCTION__,"EVENT WIFI_EVENT_STA_START");
        if(!scan){esp_wifi_connect();}
        
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
     {
        ESP_LOGD(__FUNCTION__,"EVENT WIFI_EVENT_STA_DISCONNECTED");
        if(ok == 0){
            if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
            {
                esp_wifi_connect();
                s_retry_num++;
                vTaskDelay(1000 / portTICK_PERIOD_MS);

                ESP_LOGI(__FUNCTION__, "reintentando conectar %d", s_retry_num);
            } 
            else 
            {
                WIFIstate = false;
                conf_wifi.mode = false;
                memset(&conf_wifi.ssid,0,sizeof(conf_wifi.ssid));
                memset(&conf_wifi.password,0,sizeof(conf_wifi.password) );
                //strncpy(conf_wifi.password,"",0);
                //ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_SAVE,NULL,0,portMAX_DELAY));
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            }
        }
        else
        {
            esp_wifi_connect();
            vTaskDelay(1000 * s_retry_num / portTICK_PERIOD_MS);
            ESP_LOGW(__FUNCTION__, "reintentando conectar %d",s_retry_num);
            s_retry_num++;
            if(s_retry_num > 1000){
                WIFIstate = false;
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            }
        }
        ESP_LOGW(__FUNCTION__,"Imposible conetar con AP");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ESP_LOGD(__FUNCTION__,"EVENT IP_EVENT_STA_GOT_IP");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(__FUNCTION__, "ip:" IPSTR, IP2STR(&event->ip_info.ip));
        //strncpy(conf_int_mem.ip,(const char *)&event->ip_info.ip,sizeof(ip4_addr_t));
        //ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_SAVE,NULL,0,portMAX_DELAY));
        s_retry_num = 0;
        ok = 1;
        WIFIstate = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

       
    }
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        ESP_LOGI(__FUNCTION__,"EVENT WIFI_EVENT_AP_STACONNECTED");
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(__FUNCTION__, "SoftAP %s Lista, AID=%d",MAC2STR(event->mac), event->aid);
    } 
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        ESP_LOGI(__FUNCTION__,"EVENT WIFI_EVENT_AP_STADISCONNECTED");
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(__FUNCTION__, "Soft Ap  %s eliminada, AID=%d",MAC2STR(event->mac), event->aid);
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
esp_err_t static_ip(esp_netif_t *netif)
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
        if(ip != NULL)
        {
            ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
            return ESP_FAIL;
        }
        ESP_ERR_CHECK(storage_load(NVS_TYPE_STR,"ip",ip,&ip_len));
        Ip.ip.addr = ipaddr_addr((const char*)ip);
        free(ip);

        size_t netmask_len = storage_get_size("netmask");
        if(netmask_len >0)
        {
            char *netmask = malloc(sizeof(char) * netmask_len);
            if(netmask != NULL)
            {
                ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
                return ESP_FAIL;
            }
            ESP_ERR_CHECK(storage_load(NVS_TYPE_STR,"netmask",netmask,&netmask_len));
            Ip.netmask.addr = ipaddr_addr((const char*)netmask);
            free(netmask);
        }
        size_t gateway_len = storage_get_size("netmask");
        if(gateway_len >0)
        {
            char *gateway = malloc(sizeof(char) * gateway_len);
            if(gateway != NULL)
            {
                ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
                return ESP_FAIL;
            }
            ESP_ERR_CHECK(storage_load(NVS_TYPE_STR,"netmask",gateway,&gateway_len));
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
    size_t dns1_len = storage_get_size("dns1");
    if(dns1_len > 0)
    {
        char *dns1 = malloc(sizeof(char) * dns1_len);
        if(dns1 != NULL)
        {
            ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
            return ESP_FAIL;
        }
        ESP_ERR_CHECK(storage_load(NVS_TYPE_STR,"dns1",dns1,&dns1_len));
        ESP_ERROR_CHECK(set_dns_server(netif, ipaddr_addr((const char*)dns1), ESP_NETIF_DNS_MAIN));
        free(dns1);
    }
    size_t dns2_len = storage_get_size("dns2");
    if(dns2_len > 0)
    {
        char *dns2 = malloc(sizeof(char) * dns2_len);
        if(dns2 != NULL)
        {
            ESP_LOGE(__FUNCTION__, "sin memoria dinamica :(");
            return ESP_FAIL;
        }
        ESP_ERR_CHECK(storage_load(NVS_TYPE_STR,"dns2",dns2,&dns2_len));
        ESP_ERROR_CHECK(set_dns_server(netif, ipaddr_addr((const char*)dns2), ESP_NETIF_DNS_MAIN));
        free(dns2);
    }
    return ESP_OK;
}

esp_err_t wifi_init_sta(void)
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
    strcpy((char *)wifi_config.sta.ssid, conf_wifi.ssid);
    strcpy((char *)wifi_config.sta.password, conf_wifi.password);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(__FUNCTION__, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);


    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(__FUNCTION__, "Conectado a SSID:%s",
                 conf_wifi.ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(__FUNCTION__, "Fallo al conectar a SSID:%s",
                 conf_wifi.ssid);
        return ESP_FAIL;
    } else {
        ESP_LOGE(__FUNCTION__, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}
char *wifi_scan(void)
{
    cJSON *json;
    cJSON *text;
    cJSON *msg;
    json = cJSON_CreateObject();
    msg = cJSON_CreateObject();
    scan= true;
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
esp_err_t wifi_init_softap(void)
{
    scan = true;
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

esp_err_t Wifi_start(_wifi conf_wifi)
{
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    if(conf_wifi.mode == true)
    { //
        ESP_LOGI(__FUNCTION__, "ESP_WIFI_MODE_STA");
        esp_err_t de = wifi_init_sta(conf_wifi);
        if(de == ESP_OK)
        {
            return ESP_OK;
        }
        else
        {
            ESP_ERROR_CHECK(wifi_init_softap());
        }
    }
    else
    {
        esp_err_t de = wifi_init_softap();
        if(de == ESP_OK)
        {
            return ESP_OK;
        }
        else
        {
            return ESP_FAIL;
        }
        
    }
    return ESP_OK;
}


