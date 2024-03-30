#include "dimmer.h"


static TaskHandle_t dimmer_task =NULL;
static esp_timer_handle_t _timer;
static conf_dimmer_t conf_gestor;


static void timer_callback(void* args)
{
    gpio_set_level(TRIAC,1);

}
static void IRAM_ATTR GPIO_ISR_Handler(void* arg)
{   
 
 
    esp_timer_stop(_timer);
    
    ESP_ERROR_CHECK(gpio_set_level(TRIAC,0));
 
    ESP_ERROR_CHECK(esp_timer_start_once(_timer,conf_gestor.result));
    
}

static void conf_pin(conf_dimmer_t dimmer)
{   
    gpio_config_t triac = {};
    
    triac.intr_type = GPIO_INTR_DISABLE;
    triac.mode = GPIO_MODE_OUTPUT;
    triac.pin_bit_mask = ((1ULL<<TRIAC));
    triac.pull_down_en =0;
    triac.pull_up_en = 0;
    ESP_ERROR_CHECK(gpio_config(&triac));
    ESP_ERROR_CHECK(gpio_set_level(TRIAC,0));
    gpio_config_t zero = {};
    zero.intr_type = GPIO_INTR_POSEDGE;
    zero.pin_bit_mask = ((1ULL<<ZERO));
    zero.mode = GPIO_MODE_INPUT;
    zero.pull_down_en = 0;
    zero.pull_up_en = 1;
    ESP_ERROR_CHECK(gpio_config(&zero));
    gpio_set_intr_type(ZERO, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);
}

static void dimmer_http(void *PvParams)
{
    ESP_LOGI(__FUNCTION__,"inicio de tarea dimmer");
    gpio_isr_handler_add(ZERO, GPIO_ISR_Handler, (void*)ZERO);
    esp_http_client_handle_t Inverter = http_begin(conf_gestor.inverter_url);
    int pid = 0;
    int sal =(int)Kostal_requests(Inverter);
    uint8_t count_power = 0;
    uint16_t count_send = 0;
    int NTC_temp = 0;
    int _pid = 0;
    int _ntc_pid = 0;
    int arrived = 0;
    esp_err_t err = ESP_FAIL;
    while(1)
    { 
        if(count_power >= 30)
        {
            sal =(int)Kostal_requests(Inverter);
            NTC_temp = (int)temp_termistor();
            count_power = 0;
           //ESP_LOGI(__FUNCTION__,"envio result %d min_delay %d conf_reg %d",conf_gestor.result,conf_gestor.min_delay,conf_gestor.reg);
        }
        if(count_send > KEEPALIVE_LAP)
        {
            char *buff =(char*)pvPortMalloc(sizeof(int));
            conf_gestor.level = map(conf_gestor.result,10000,conf_gestor.min_delay,0,100);
            itoa(conf_gestor.level,buff,10);
            queue_send(DIMMER_TX,buff,"level",20/portTICK_PERIOD_MS);
            vPortFree(buff);
            
            ESP_LOGI(__FUNCTION__,"envio potencia %d",conf_gestor.level);
            count_send = 0;
        }
        if(NTC_temp > 50)
        {
            Fan_state(1);
        }
        else if(NTC_temp > 55)
        {
             
            _pid = PID(LIMIT_TEMP,NTC_temp,&conf_gestor.pid_NTC);
            _ntc_pid = map(_pid,-5,5,-10,10);
            conf_gestor.min_delay +=_ntc_pid;
            ESP_LOGW(__FUNCTION__,"temperatura escesiva!");
            ESP_LOGI(__FUNCTION__,"inicio pid temperatura");

        }
        else if(NTC_temp < 45 && NTC_temp > LIMIT_TEMP)
        {
            _pid = PID(LIMIT_TEMP,NTC_temp,&conf_gestor.pid_NTC);
            _ntc_pid = map(_pid,-5,5,-10,10);
            conf_gestor.min_delay +=_ntc_pid;
            if(conf_gestor.min_delay < 100)
            {
                conf_gestor.min_delay = 100;
            }
            Fan_state(0);
        }
        else
        {
            conf_gestor.min_delay = 100;
        }
        
        pid = PID(1-conf_gestor.reg,sal,&conf_gestor.pid_Pwr);
        arrived = map(pid,-1000,1000,-500,500);
        sal += arrived;
        conf_gestor.result +=pid;
        if(conf_gestor.min_delay > 10000)
        {
            conf_gestor.min_delay = 10000;
        }
        if(conf_gestor.result <  conf_gestor.min_delay)
        {
            conf_gestor.result =  conf_gestor.min_delay;
            conf_gestor.pid_Pwr.CumError = 0;
        }
        
        if(conf_gestor.result > 10000)
        {
            conf_gestor.result = 10000;
            conf_gestor.pid_Pwr.CumError = 0;
            conf_gestor._enable =false;
        }
        else
        {
            conf_gestor._enable = true;
        }
        msg_queue_t *msg = (msg_queue_t*)pvPortMalloc(sizeof(msg_queue_t));
        ESP_MALLOC_CHECK(msg);
        err = queue_receive(DIMMER_RX,100/portTICK_PERIOD_MS,msg);
        if(err == ESP_OK)
        {
            if(msg->len_msg > 0 && strcmp(msg->topic,"dimmer") == 0)
            {   
                int calc = map(atoi(msg->msg),0,100,0,MAX_POWER); // pasamos de % a watios 
                conf_gestor.reg = calc;
                ESP_LOGW(__FUNCTION__,"nuevo nivel = %d",calc);
            }
            else if(msg->len_msg > 0 && strcmp(msg->topic,"temperatura")== 0)
            {

                printf("temperatura = %f\n",atof(msg->msg));
                /*implementación pendiente*/
            }
            else if(msg->len_msg > 0 && strcmp(msg->topic,"kp")== 0)
            {
            
                conf_gestor.pid_Pwr.Kp = atof(msg->msg);
                ESP_LOGI(__FUNCTION__,"nuevo valor kp = %f",conf_gestor.pid_Pwr.Kp);
            }
            else if(msg->len_msg > 0 && strcmp(msg->topic,"ki")== 0)
            {
                conf_gestor.pid_Pwr.Ki = atof(msg->msg);
            }
            else if(msg->len_msg > 0 && strcmp(msg->topic,"kd")== 0)
            {
                conf_gestor.pid_Pwr.Kd = atof(msg->msg);
            }
            else if(msg->len_msg > 0 && strcmp(msg->topic,"min")== 0)
            {
                conf_gestor.pid_Pwr.min = (int)atof(msg->msg);
            }
            else if(msg->len_msg > 0 && strcmp(msg->topic,"max")== 0)
            {
                conf_gestor.pid_Pwr.max = (int)atof(msg->msg);
            }
        }
        vPortFree(msg);
        vTaskDelay(50/portTICK_PERIOD_MS);
        count_power ++;
        count_send ++;
    }
}
void dimmer_init(void)
{   
   
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
        conf_gestor.pid_Pwr.Kp = (float)0.5;
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
        conf_gestor.pid_Pwr.Ki =(float)0.2;
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
        conf_gestor.pid_Pwr.Kd = (float)0.8;
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
        ESP_LOGI(__FUNCTION__,"Url Inverter= %s",conf_gestor.inverter_url);
        free(url);
    }
    else
    {
        abort();
    }
    conf_gestor.min_delay = 100;
    conf_gestor._enable = true;
    conf_gestor.pid_Pwr.CumError = 0;
    conf_gestor.pid_Pwr.LastError = 0;

    conf_gestor.result = 10000;
    conf_gestor.reg = 0;
    conf_gestor.level = 0;
    conf_gestor.pid_NTC.Kp = 0.3;
    conf_gestor.pid_NTC.Ki = 0.2;
    conf_gestor.pid_NTC.Kd = 0.3;
    conf_gestor.pid_NTC.CumError =0;
    conf_gestor.pid_NTC.Error = 0;
    conf_gestor.pid_NTC.LastError = 0;
    conf_gestor.pid_NTC.max = 5;
    conf_gestor.pid_NTC.min = -5;
    const esp_timer_create_args_t timer_args = {
            .callback = timer_callback,
            .name = "timmer"
            
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &_timer));
    //conf_timmer(conf_gestor);
    conf_pin(conf_gestor);
    xTaskCreate(&dimmer_http,"dimmer",4000,NULL,3,NULL);
    //ESP_ERROR_CHECK(task_create(&dimmer_http,"dimmer",4,NULL));
}
void dimmer_stop(void)
{
    if(dimmer_task != NULL)
    {
        eTaskState state = eTaskGetState(&dimmer_task);
        ESP_LOGW(__FUNCTION__,"dimmer task state %d",(int)state);
        if(state != eInvalid && state != eDeleted)
        {
            vTaskDelete(&dimmer_task);
            gpio_isr_handler_remove(ZERO);
            esp_timer_stop(_timer);
            ESP_LOGW(__FUNCTION__,"Dimmer task stop");

        }
        else
        {
            ESP_LOGE(__FUNCTION__,"dimmer task invalid state :(");
        }

    }

    
}
