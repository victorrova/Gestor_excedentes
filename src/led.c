
#include "led.h"

static uint8_t led_strip_pixels[LED_NUMBERS * 3];
extern EventGroupHandle_t Bits_events;
color_led_t color = {100,100,0};
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

esp_err_t led_init(void)
{
    esp_err_t err =ESP_FAIL;
    color_led_t color = {0};
    rmt_channel_handle_t led_chan = 0;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, 
        .gpio_num = RMT_LED_STRIP_GPIO_NUM,
        .mem_block_symbols = 64, 
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, 
    };
    err = rmt_new_tx_channel(&tx_chan_config, &led_chan);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] rmt tx Channel Fail");
        return ESP_FAIL;
    }
    rmt_encoder_handle_t led_encoder = NULL;
    led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
    };
    err = rmt_new_led_strip_encoder(&encoder_config, &led_encoder);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] Install led strip encoder fail");
        return ESP_FAIL;
    }
    err = rmt_enable(led_chan);
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[FAIL] IEnable RMT TX channel");
        return ESP_FAIL;
    }
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, 
    };
    led_strip_pixels[0] = 255;
    led_strip_pixels[1]= 255;
    led_strip_pixels[2] = 0;
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
    return err;

}
