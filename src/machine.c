#include "machine.h"




static esp_adc_cal_characteristics_t adc1_chars;


#define FAN 19



int _free_mem(void)
{
    int a =  esp_get_free_heap_size()/1024;
    ESP_LOGW(__FUNCTION__, "[APP] Free memory: %d Kbytes", a);
    return a;  
}

void termistor_init(void)
{
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_DEFAULT, 1100, &adc1_chars);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0));
}



void Fan_init(void)
{
    gpio_config_t fan = {};
    fan.pin_bit_mask = (1ULL<<FAN);
    fan.intr_type = GPIO_INTR_DISABLE;
    fan.mode = GPIO_MODE_OUTPUT;
    fan.pull_down_en = 0;
    fan.pull_up_en = 0;
    ESP_ERROR_CHECK(gpio_config(&fan));
    ESP_LOGI(__FUNCTION__,"prueba de ventilador...");
    gpio_set_level(FAN,1);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    gpio_set_level(FAN,0);
}

void Fan_state(int state)
{
    if(state == 1)
    {
        ESP_LOGI(__FUNCTION__," Fan on!");
        ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(FAN,1));
    }
    else if(state == 0)
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

