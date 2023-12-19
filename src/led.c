
#include "led.h"



static led_params_t led;


void led_machine_ok(void)
{
    led.color.blue = 0;
    led.color.green = 0;
    led.color.red = 0;
    while(led.color.blue < 254)
    {
        led.pixel[0] = led.color.green;
        led.pixel[1] = led.color.red;
        led.pixel[2] = led.color.blue;
        ESP_ERROR_CHECK(rmt_transmit(led.led_chan, led.led_encoder, led.pixel, sizeof(led.pixel), &led.tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led.led_chan, portMAX_DELAY));
        led.color.blue += 5;
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}


void led_total_connect(void)
{
    led.color.blue = 0;
    led.color.green = 0;
    led.color.red = 0;
    while(led.color.green < 254)
    {
        led.pixel[0] = led.color.green;
        led.pixel[1] = led.color.red;
        led.pixel[2] = led.color.blue;
        ESP_ERROR_CHECK(rmt_transmit(led.led_chan, led.led_encoder, led.pixel, sizeof(led.pixel), &led.tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led.led_chan, portMAX_DELAY));
        led.color.green += 5;
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
    
}

void led_on_message(void)
{


        led.pixel[0] = 0;
        led.pixel[1] = 0;
        led.pixel[2] = 0;
        ESP_ERROR_CHECK(rmt_transmit(led.led_chan, led.led_encoder, led.pixel, sizeof(led.pixel), &led.tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led.led_chan, portMAX_DELAY));
        vTaskDelay(100/portTICK_PERIOD_MS);
        led.pixel[0] = led.color.red;
        led.pixel[1] = led.color.green;
        led.pixel[2] = led.color.blue;
        ESP_ERROR_CHECK(rmt_transmit(led.led_chan, led.led_encoder, led.pixel, sizeof(led.pixel), &led.tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led.led_chan, portMAX_DELAY));
        
}

void led_fail(void)
{
        led.pixel[0] = 0;
        led.pixel[1] = 254;
        led.pixel[2] = 0;
        ESP_ERROR_CHECK(rmt_transmit(led.led_chan, led.led_encoder, led.pixel, sizeof(led.pixel), &led.tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led.led_chan, portMAX_DELAY));
        
}

void led_off(void)
{
        led.pixel[0] = 0;
        led.pixel[1] = 0;
        led.pixel[2] = 0;
        ESP_ERROR_CHECK(rmt_transmit(led.led_chan, led.led_encoder, led.pixel, sizeof(led.pixel), &led.tx_config));
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(led.led_chan, portMAX_DELAY));
}
esp_err_t led_init(void)
{
    esp_err_t err =ESP_FAIL;
    led.led_chan = 0;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, 
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, 
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, 
    };
    err = rmt_new_tx_channel(&tx_chan_config, &led.led_chan);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] rmt tx Channel Fail");
        return ESP_FAIL;
    }
    led.led_encoder = NULL;
    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    err = rmt_new_led_strip_encoder(&encoder_config, &led.led_encoder);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] Install led strip encoder fail");
        return ESP_FAIL;
    }
    err = rmt_enable(led.led_chan);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] IEnable RMT TX channel");
        return ESP_FAIL;
    }
    led.tx_config.loop_count = 0;
    led.pixel[0] = 0;
    led.pixel[1]= 0;
    led.pixel[2] = 0;
    ESP_ERROR_CHECK(rmt_transmit(led.led_chan, led.led_encoder, led.pixel, sizeof(led.pixel), &led.tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led.led_chan, portMAX_DELAY));
    return err;
}

