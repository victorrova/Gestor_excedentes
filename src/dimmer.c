#include "dimmer.h"

#define ZERO 18 
#define TRIAC 14 



static void timer_callback(void* args)
{
    gpio_set_level(TRIAC,1);

}
static void IRAM_ATTR GPIO_ISR_Handler(void* arg)
{   
    conf_dimmer_t gestor =*(conf_dimmer_t*) arg;
    esp_timer_stop(gestor._timer);
    gpio_set_level(TRIAC,0);
    if(gestor._enable == true)
    {
        ESP_ERROR_CHECK(esp_timer_start_once(gestor._timer, gestor.result));
    }
    
}

static void conf_timmer(conf_dimmer_t dimmer)
{
    const esp_timer_create_args_t timer_args = {
            .callback = timer_callback,
            .name = "timmer"
            
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &dimmer._timer));
}
static void conf_pin(conf_dimmer_t dimmer)
{   
    gpio_config_t triac = {};
    triac.pin_bit_mask = (1ULL<<TRIAC);
    triac.intr_type = GPIO_INTR_DISABLE;
    triac.mode = GPIO_MODE_OUTPUT;
    triac.pull_down_en =0;
    triac.pull_up_en = 0;
    ESP_ERROR_CHECK(gpio_config(&triac));

    gpio_config_t zero = {};
    zero.pin_bit_mask = (1ULL<<ZERO);
    zero.mode = GPIO_MODE_INPUT;
    zero.pull_down_en = GPIO_PULLDOWN_ONLY;
    zero.pull_up_en = 0;
    zero.intr_type = GPIO_INTR_POSEDGE;
    ESP_ERROR_CHECK(gpio_config(&zero));
    gpio_install_isr_service(0);
    gpio_isr_handler_add(ZERO, GPIO_ISR_Handler, &dimmer);


}

static void dimmer_http(void *PvParams)
{
    ESP_LOGI(__FUNCTION__,"inicio de tarea dimmer");
    conf_dimmer_t conf_gestor = *(conf_dimmer_t*)PvParams;
    esp_http_client_handle_t Inverter = http_begin(conf_gestor.inverter_url);
    int pid = 0;
    int sal =(int)Kostal_requests(Inverter);
    uint8_t count = 0;
    int NTC_temp = 0;
    msg_queue_t msg;
    while(1)
    { 
        if(count == 30)
        {
            sal =(int)Kostal_requests(Inverter);
            NTC_temp = (int)temp_termistor();
            char reg[10];
            conf_gestor.level = map(conf_gestor.reg,conf_gestor.min_delay,10000,0,100);
            itoa(conf_gestor.level,reg,10);
            queue_send(DIMMER_TX,reg,"level",10);
        }
        if(count >30)
        {
            count = 0;
        }
        if(NTC_temp > 60)
        {
            Fan_state(1);
        }
        else if(NTC_temp > 65)
        {
            int _pid = PID(50,NTC_temp,&conf_gestor.pid_NTC);
            int _ntc_pid = map(_pid,-5,5,-50,50);
            conf_gestor.min_delay +=_ntc_pid;

        }
        else if(NTC_temp < 55)
        {
            int _pid = PID(55,NTC_temp,&conf_gestor.pid_NTC);
            int _ntc_pid = map(_pid,-5,5,-50,50);
            conf_gestor.min_delay +=_ntc_pid;
            if(conf_gestor.min_delay < 100)
            {
                conf_gestor.min_delay = 100;
            }
            Fan_state(0);
        }
        pid = PID(1-conf_gestor.reg,sal,&conf_gestor.pid_Pwr);
        int arrived = map(pid,-1000,1000,-500,500);
        sal += arrived;
        conf_gestor.result +=pid;

        if(conf_gestor.result < conf_gestor.min_delay)
        {
            conf_gestor.result = conf_gestor.min_delay;
            conf_gestor.pid_Pwr.CumError = 0;
        }
        
        else if(conf_gestor.result > 10000)
        {
            conf_gestor.result = 10000;
            conf_gestor.pid_Pwr.CumError = 0;
            conf_gestor._enable =false;
        }
        else
        {
            conf_gestor._enable = true;
        }
        msg = queue_receive(DIMMER_RX,20);
        if(msg.len_msg > 0 && strcmp(msg.topic,"dimmer") == 0)
        {   
            
            int calc = map(atoi(msg.msg),0,100,0,3600); // pasamos de % a watios 
            conf_gestor.reg = calc;
        }
        else if(msg.len_msg > 0 && strcmp(msg.topic,"temperatura")== 0)
        {

            printf("temperatura = %f\n",atof(msg.msg));
            /*implementacion pendiente*/
        }
        else if(msg.len_msg > 0 && strcmp(msg.topic,"kp")== 0)
        {
            conf_gestor.pid_Pwr.Kp = atof(msg.msg);
        }
        else if(msg.len_msg > 0 && strcmp(msg.topic,"ki")== 0)
        {
            conf_gestor.pid_Pwr.Ki = atof(msg.msg);
        }
        else if(msg.len_msg > 0 && strcmp(msg.topic,"kd")== 0)
        {
            conf_gestor.pid_Pwr.Kd = atof(msg.msg);
        }
        else if(msg.len_msg > 0 && strcmp(msg.topic,"min")== 0)
        {
            conf_gestor.pid_Pwr.min = (int)atof(msg.msg);
        }
        else if(msg.len_msg > 0 && strcmp(msg.topic,"max")== 0)
        {
            conf_gestor.pid_Pwr.max = (int)atof(msg.msg);
        }
        
        vTaskDelay(100/portTICK_PERIOD_MS);
        count++;
    }
}
void dimmer_init(void)
{   
    conf_dimmer_t conf_gestor;
    union float_converter converter;
    size_t kp_len = storage_get_size("kp");
    if( kp_len > 0)
    {
        uint32_t kp = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"kp",kp,NULL));
        
        converter.ui = kp;
       
        conf_gestor.pid_Pwr.Kp = converter.fl;
        ESP_LOGI(__FUNCTION__,"Nvs Kp = %f",converter.fl);
    }
    else
    {
        conf_gestor.pid_Pwr.Kp = 0.5;
        ESP_LOGW(__FUNCTION__,"Nvs default Kp");
    }
    size_t ki_len = storage_get_size("ki");
    if( ki_len > 0)
    {
        uint32_t ki = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"ki",ki,NULL));

        converter.ui = ki;
         conf_gestor.pid_Pwr.Ki = converter.fl;
          ESP_LOGI(__FUNCTION__,"Nvs Ki = %f",converter.fl);
    }
    else
    {
        conf_gestor.pid_Pwr.Ki = 0.2;
        ESP_LOGW(__FUNCTION__,"Nvs default Ki");
    }
    size_t kd_len = storage_get_size("kd");
    if( kd_len > 0)
    {
        uint32_t kd = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"kd",kd,NULL));
        converter.ui = kd;
        conf_gestor.pid_Pwr.Kd = converter.fl;
        ESP_LOGI(__FUNCTION__,"Nvs Kd = %f",converter.fl);
    }
    else
    {
        conf_gestor.pid_Pwr.Kd = 0.8;
        ESP_LOGW(__FUNCTION__,"Nvs default Kd");
    }
    size_t pid_max_len = storage_get_size("pid_max");
    if(pid_max_len > 0)
    {
        uint32_t pid_max = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"pid_max",pid_max,NULL));
        conf_gestor.pid_Pwr.max= pid_max;
        ESP_LOGI(__FUNCTION__,"Nvs max pid value  = %lu",pid_max);
    }
    else
    {
        conf_gestor.pid_Pwr.max= 1000;
        ESP_LOGW(__FUNCTION__,"Nvs default max pid");
    }
    size_t pid_min_len = storage_get_size("pid_min");
    if(pid_min_len > 0)
    {
        uint32_t pid_min = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"pid_min",pid_min,NULL));
        conf_gestor.pid_Pwr.min= 1 - pid_min;
        ESP_LOGI(__FUNCTION__,"Nvs min pid value  = %lu",pid_min);
    }
    else
    {
        conf_gestor.pid_Pwr.min= -1000;
        ESP_LOGW(__FUNCTION__,"Nvs default min pid");
    }
    size_t url_len = storage_get_size("url_inverter");
    if(url_len >0)
    {
        char *url = (char*)malloc(sizeof(char) * url_len);
        ESP_MALLOC_CHECK(url);
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"url_inverter",url,&url_len));
        memcpy(conf_gestor.inverter_url,url,url_len);
        free(url);
    }
    else
    {
        abort();
    }
    conf_gestor.min_delay = 100;
    conf_gestor._enable = false;
    conf_gestor.pid_Pwr.CumError = 0;
    conf_gestor.pid_Pwr.LastError = 0;
    conf_gestor.result = 10000;
    conf_gestor.reg = 0;
    conf_gestor.level = 0;
    conf_gestor.pid_NTC.Kp = 0.5;
    conf_gestor.pid_NTC.Ki = 0.2;
    conf_gestor.pid_NTC.Kd = 0.6;
    conf_gestor.pid_NTC.CumError =0;
    conf_gestor.pid_NTC.Error = 0;
    conf_gestor.pid_NTC.LastError = 0;
    conf_gestor.pid_NTC.max = 5;
    conf_gestor.pid_NTC.min = -5;
    conf_timmer(conf_gestor);
    conf_pin(conf_gestor);
    xTaskCreate(&dimmer_http,"DIMMER",6600,&conf_gestor,4,NULL);
}

    
