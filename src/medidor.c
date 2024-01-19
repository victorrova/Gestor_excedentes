
#include "medidor.h"


esp_err_t Hlw8032_Init(void)
{
    esp_err_t err;

    const uart_config_t uart_config = {
    .baud_rate = 4800,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_EVEN,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_APB,
    };
    
    err = uart_driver_install(UART_PORT, BUFFER, 0, 0, NULL, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "[FAIL] uart driver install");
        return err;
    }
    err = uart_param_config(UART_PORT, &uart_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "UART param config failed");
        return err;
    }
    uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__, "UART pin setting failed");
        return err;
    }
    return err;
}

esp_err_t Hlw8032_read(meter_t *meter)
{
    
    int rxBytes = 0;
    uint8_t *buffer = NULL;
    size_t data_len = 0;
    uart_flush(UART_PORT);
    do
    {
        uart_get_buffered_data_len(UART_PORT, &data_len);
    } while ((int)data_len < 24);
    
    if((int)data_len >=24)
    {
        uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t)*50);
        ESP_MALLOC_CHECK(buffer);
        rxBytes = uart_read_bytes(UART_PORT, buffer, 24, 60/portTICK_PERIOD_MS);
        uart_flush(UART_PORT);
        switch (buffer[0])
        {
        case 0X5A:
            /* code */
            break;
        case 0xf2:
            break;
        default:

            break;
        }
    }
    else
    {
        ESP_LOGE(__FUNCTION__, "buffer incompleto = %d",(int)data_len);
        uart_flush(UART_PORT);
        return ESP_FAIL;
    }
    if(buffer !=NULL)
    {
        free(buffer);
    }
    return ESP_OK;
}