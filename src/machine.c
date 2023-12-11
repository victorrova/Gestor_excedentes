#include "machine.h"




static esp_adc_cal_characteristics_t adc1_chars;


#define FAN 19


static hlw8032_t  hlw_meter;


esp_err_t Meter_init(void)
{
   esp_err_t err = ESP_FAIL;
   err = hlw8032_serial_begin(&hlw_meter,2,16,256);
   hlw8032_set_I_coef_from_R(&hlw_meter, 0.001);
   hlw8032_set_V_coef_from_R(&hlw_meter, 1880000, 1000);
   return err;
}
int _free_mem(void)
{
    int a =  esp_get_free_heap_size()/1024;
    ESP_LOGW(__FUNCTION__, "[APP] Free memory: %d Kbytes", a);
    return a;  
}

esp_err_t termistor_init(void)
{
    esp_err_t err = ESP_FAIL;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_DEFAULT, 1100, &adc1_chars);
    err = adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] adc config ");
        return ESP_FAIL;
    }
    err = adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] adc channel config ");
        return ESP_FAIL;
    }
    return err;
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
    //int raw = (int)esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_6), &adc1_chars);
    int adc = adc1_get_raw(ADC1_CHANNEL_6);
    float celsius = 1 / (log(1 / (4095. / adc - 1)) / 3950 + 1.0 / 298.15) - 273.15;
    ESP_LOGW(__FUNCTION__, "[APP] temperatura interna %f", celsius);
    return celsius;

}
static int mqtt_logger(const char *msg, va_list arg)
{
    char buffer[512];
    vsprintf(buffer,msg,arg);
    esp_err_t result = queue_send(MQTT_TX, buffer,"/casa/gestor/log",portMAX_DELAY);
    if(result == -1){
        return -1;
    }
    return vprintf(msg,arg);

}
static int ws_logger(const char *msg, va_list arg)
{
    char buffer[512];
    vsprintf(buffer,msg,arg);
    esp_err_t result = queue_send(WS_TX,buffer,"/ws/logger",portMAX_DELAY);
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


void timer_init(s_timer_t *param,int prescaler,int timer,int (*callback)(void), void *params)
{
    param->count =0;
    param->prescaler=prescaler;
    param->timer = timer;
    param->function_cb= callback;
    param->params = params;
    param->result = 0;
}

void timer_loop(s_timer_t *param)
{
    int ciclo = param->timer / param->prescaler;
    if(ciclo <= param->count)
    {
        int result =param->function_cb();
        param->result = result;
        param->count =0;
    }
    else
    {
        param->count++;
    }

}


int Keepalive(void)
{
    cJSON *keep = cJSON_CreateObject();
    cJSON *root = cJSON_CreateObject();
    int state_gestor = 0;
    float temp = 0.0;
    float voltage = 0.0;
    float intensidad = 0.0 ;
    float P_activa = 0.0;
    float P_appa = 0.0;
    float factor_p = 0.0;
    esp_err_t err = ESP_FAIL;

    msg_queue_t msg = queue_receive(DIMMER_TX,100);
    if(msg.len_msg >0 && strcmp(msg.topic,"level")== 0)
    {
        state_gestor =atoi(msg.msg);
        ESP_LOGD(__FUNCTION__,"nuevo nivel = %d",state_gestor);
    }
    temp = temp_termistor();
    for(int i = 0; i == 10;i++)
    {
        err = hlw8032_read(&hlw_meter);
        if(err == ESP_OK)
        {
            break;
        }
        else{
            vTaskDelay(50/portTICK_PERIOD_MS);
        }
    }
    if(err == ESP_OK)
    {
        voltage = hlw8032_get_V(&hlw_meter);
        intensidad = hlw8032_get_I(&hlw_meter);
        P_activa = hlw8032_get_P_active(&hlw_meter);
        P_appa = hlw8032_get_P_apparent(&hlw_meter);
        factor_p = hlw8032_get_P_factor(&hlw_meter);

    }
    else
    {
        ESP_LOGW(__FUNCTION__,"HLW8032 error de lectura");
    }
    cJSON_AddNumberToObject(root,"temp_ntc",temp);
    cJSON_AddNumberToObject(root,"voltage",voltage);
    cJSON_AddNumberToObject(root,"current",intensidad);
    cJSON_AddNumberToObject(root,"p_activa",P_activa);
    cJSON_AddNumberToObject(root,"p_appa",P_appa);
    cJSON_AddNumberToObject(root,"factor_p",factor_p);
    cJSON_AddItemToObject(keep, "keepalive",root);
    char *msg_root = cJSON_Print(keep);
    queue_send(MQTT_TX,msg_root,NULL,portMAX_DELAY);
    cJSON_Delete(keep);
    ESP_LOGD(__FUNCTION__,"keep alive enviado!");
    return ESP_OK;
}