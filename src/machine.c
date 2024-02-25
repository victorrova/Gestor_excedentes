#include "machine.h"




static esp_adc_cal_characteristics_t adc1_chars;
extern EventGroupHandle_t Bits_events;

esp_err_t Meter_init(void)
{

   return Hlw8032_Init();
}


esp_err_t termistor_init(void)
{
    esp_err_t err = ESP_FAIL;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 1100, &adc1_chars);
    err = adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] adc config ");
        return ESP_FAIL;
    }
    err = adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_11 );
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] adc channel config ");
        return ESP_FAIL;
    }
    return err;
}

uint32_t free_mem(void)
{
   return esp_get_free_heap_size() ;
}

esp_err_t Fan_init(void)
{
    esp_err_t err = ESP_FAIL;
    gpio_config_t fan = {};
    fan.pin_bit_mask = (1ULL<<FAN);
    fan.intr_type = GPIO_INTR_DISABLE;
    fan.mode = GPIO_MODE_INPUT_OUTPUT;
    fan.pull_down_en = 0;
    fan.pull_up_en = 0;
    err = gpio_config(&fan);
    ESP_LOGI(__FUNCTION__,"prueba de ventilador...");
    gpio_set_level(FAN,1);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    gpio_set_level(FAN,0);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    return err;
}
void Fan_state(int state)
{
    int level = gpio_get_level(FAN);
    if(state == 1 && level != 1)
    {
        ESP_LOGI(__FUNCTION__," Fan on!");
        ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(FAN,1));
    }
    else if(state == 0 && level != 0)
    {
        ESP_LOGI(__FUNCTION__," Fan off!");
        ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(FAN,0));
    } 
}
 

float temp_termistor(void)
{
    double R1 = 10000;
    double logR2, R2, T, Tc;
    double c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
    int Vo = adc1_get_raw(ADC1_CHANNEL_6);
    R2 = R1 * (4095.0 / (float)Vo - 1.0);
    logR2 = log(R2);
    T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
    Tc = T - 273.15;
    return (float)Tc;

}


static int mqtt_logger(const char *msg, va_list arg)
{
    char buffer[256];
    vsprintf(buffer,msg,arg);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root,"logger",buffer);
    char* buff = cJSON_Print(root);
    esp_err_t result = queue_send(MQTT_TX, buff,"NONE",portMAX_DELAY);
    cJSON_Delete(root);
    if(buff != NULL)
    {
        free(buff);
    }
    if(result == -1){
        
        return -1;
    }

    return vprintf(msg,arg);

}
static int ws_logger(const char *msg, va_list arg)
{
    char buffer[256];
    vsprintf(buffer,msg,arg);
    esp_err_t result = queue_send(WS_TX,buffer,"NONE",portMAX_DELAY);
    if(result == -1){
        return -1;
    }
    return vprintf(msg,arg);

}

static int Oled_logger(const char *msg,va_list arg)
{
    char buffer[256];
    vsprintf(buffer,msg,arg);
    esp_err_t result = queue_send(OLED_TX,buffer,"oled_loggger",portMAX_DELAY);
    if(result == -1){
        return -1;
    }
    return vprintf(msg,arg);


}
void set_stream_logger(int logger)
{
    if( logger == MQTT_TX)
    {
        esp_log_set_vprintf(&mqtt_logger);
        ESP_LOGW(__FUNCTION__,"log via mqtt seleccionado");
    }
    else if(logger == WS_TX)
    {
        esp_log_set_vprintf(&ws_logger);
        ESP_LOGW(__FUNCTION__,"log via websocket seleccionado");
    }
    else if(logger == OLED_TX)
    {
        esp_log_set_vprintf(&Oled_logger);
        ESP_LOGW(__FUNCTION__,"log via Oled seleccionado");

    }
    else
    {
       esp_log_set_vprintf(&vprintf); 
    }
}

esp_err_t Keepalive(int state_gestor, char *exit)
{ 
    int temp = 0;
    uint32_t mem = free_mem();
    temp = temp_termistor();
    esp_err_t err = ESP_FAIL;
    meter_t *met = (meter_t*)pvPortMalloc(sizeof(meter_t));
    for(int i = 0; i < 10;i++)
    {
        err =Hlw8032_Read(met);
        if(err == ESP_OK)
        {
            break;
        }
        else
        {
            vTaskDelay(250/portTICK_PERIOD_MS);
        }
    }
    if(err != ESP_OK)
    {
       
        ESP_LOGW(__FUNCTION__,"HLW8032 error de lectura");
        return ESP_FAIL;
    }
    cJSON *keep = cJSON_CreateObject();
    cJSON *root = cJSON_CreateObject();
    cJSON *v = cJSON_CreateNumber(met->Voltage);
    cJSON *i = cJSON_CreateNumber(met->Current);
    cJSON *t = cJSON_CreateNumber(temp);
    cJSON *pa = cJSON_CreateNumber(met->Power_active);
    cJSON *pap = cJSON_CreateNumber(met->Power_appa);
    cJSON *st = cJSON_CreateNumber(state_gestor);
    cJSON *me = cJSON_CreateNumber(mem);
    cJSON_AddItemToObject(keep, "keepalive",root);
    cJSON_AddItemToObject(root,"temp_ntc",t);
    cJSON_AddItemToObject(root,"voltage",v);
    cJSON_AddItemToObject(root,"current",i);
    cJSON_AddItemToObject(root,"p_activa",pa);
    cJSON_AddItemToObject(root,"p_appa",pap);
    cJSON_AddItemToObject(root,"state",st);
    cJSON_AddItemToObject(root,"free_mem",me);
    cJSON_PrintPreallocated(keep,exit,MAX_PAYLOAD,0);
    cJSON_Delete(keep);
    vPortFree(met);
    return ESP_OK;
}

static void IRAM_ATTR ISR_ap_call(void* arg)
{
    xEventGroupSetBits(Bits_events,WIFI_CHANGE);
}
 
esp_err_t Ap_call_Init(void)
{
    esp_err_t err = ESP_FAIL;
    gpio_config_t select = {};
    select.pin_bit_mask = ((1ULL<<SELECT));
    select.intr_type = GPIO_INTR_NEGEDGE;
    select.mode = GPIO_MODE_INPUT;
    select.pull_down_en = 0;
    select.pull_up_en = 1;
    err = gpio_config(&select);
    err = gpio_set_intr_type(SELECT, GPIO_INTR_ANYEDGE);
    err = gpio_install_isr_service(0);
    err = gpio_isr_handler_add(SELECT,ISR_ap_call, (void*)SELECT);
    return err;
}