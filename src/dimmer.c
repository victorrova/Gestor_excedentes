#include "dimmer.h"

#define ZERO 18 
#define TRIAC 14 

conf_dimmer_t conf_gestor;

static void timer_callback(void* args)
{
    gpio_set_level(TRIAC,1);

}
static void IRAM_ATTR GPIO_ISR_Handler(void* arg)
{   
    esp_timer_stop(conf_gestor._timer);
    gpio_set_level(TRIAC,0);
    if(conf_gestor._enable == true)
    {
        ESP_ERROR_CHECK(esp_timer_start_once(conf_gestor._timer, conf_gestor.result));
    }
    
}

static void conf_timmer(void)
{
    const esp_timer_create_args_t timer_args = {
            .callback = timer_callback,
            .name = "timmer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &conf_gestor._timer));
}
static void conf_pin(void)
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
    gpio_isr_handler_add(ZERO, GPIO_ISR_Handler, (void*) ZERO);


}

static void dimmer_http(void *PvParams)
{
    ESP_LOGI(__FUNCTION__,"inicio de tarea dimmer");
    esp_http_client_handle_t Inverter = http_begin(conf_gestor.inverter_url);
    int pid = 0;
    int sal =0;
    uint8_t count = 0;
    msg_queue_t msg;
    while(1)
    { 
        if(count == 30)
        {
            sal =(int)Kostal_requests(Inverter);
            
        }
        if(count >30){count = 0;}
        pid = PID(1-conf_gestor.reg,sal,&conf_gestor.pid);
        int arrived = map(pid,-1000,1000,-500,500);
        sal += arrived;
        conf_gestor.result +=pid;
        if(conf_gestor.result < conf_gestor.min_delay)
        {
            conf_gestor.result = conf_gestor.min_delay;
            conf_gestor.pid.CumError = 0;
            //conf_gestor._enable = false;
        }
        
        else if(conf_gestor.result > 10000)
        {
            conf_gestor.result = 10000;
            conf_gestor.pid.CumError = 0;
            conf_gestor._enable =false;
        }
        else
        {
            conf_gestor._enable = true;
        }
        msg = queue_receive(DIMMER,20);
        if(msg.len_msg > 0)
        {   
            
            int calc = map(atoi(msg.msg),0,100,0,3600); // pasamos de % a watios 
            conf_gestor.reg = calc;
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
        count++;
    }
}
void dimmer_task(void)
{   
    conf_gestor.min_delay = 100;
    conf_gestor.result = 10000;
    conf_gestor.NTC_Temp = 0;
    conf_gestor._enable = false;
    conf_gestor.pid.Kp = 0.5;
    conf_gestor.pid.Ki = 0.2;
    conf_gestor.pid.Kd = 0.8;
    conf_gestor.pid.CumError = 0;
    conf_gestor.pid.LastError = 0;
    conf_gestor.pid.max= 1000;
    conf_gestor.pid.min = -1000;
    conf_gestor.result = 10000;
    conf_gestor.reg = 0;
    conf_timmer();
    conf_pin();
    xTaskCreate(&dimmer_http,"DIMMER",1024*4,NULL,4,NULL);
}

    
