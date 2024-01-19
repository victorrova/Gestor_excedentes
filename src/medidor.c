
#include "medidor.h"


static bool Checksum(int position,uint8_t *buffer)
{
    uint8_t checksum = 0;
    for(uint8_t a = position + 2; a<= position + 22; a++)
    {
        checksum = checksum + buffer[a];
    }
    if (checksum != buffer[position + 23])
    {
        return false;
    }
    return true;
}

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
    int position = 0;
    uart_flush(UART_PORT);
    do
    {
        uart_get_buffered_data_len(UART_PORT, &data_len);
    } while ((int)data_len < 64);
    if((int)data_len >=64)
    {
        uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t)*128);
        ESP_MALLOC_CHECK(buffer);
        rxBytes = uart_read_bytes(UART_PORT, buffer, 64, 60/portTICK_PERIOD_MS);
        uart_flush(UART_PORT);
#ifdef DEBUG
        for(uint8_t i = 0; i<rxBytes;i++)
        {
            printf("%02x ",buffer[i]);
        }
        printf("...%d\n Bytes\n",rxBytes);
#endif
        for(uint8_t i=0;i<rxBytes;i++)
        {
            if((buffer[i]  >= 0xf0 && buffer[i+1] == 0x5a) || (buffer[i] == 0x55 && buffer[i+1] == 0x5a))
            {
                if(i <48)
                {
                    position = i;
                    break;
                }
                position = -1;
                break;
            }
        }
        if(position < 0)
        {
            ESP_LOGE(__FUNCTION__,"order buffer incorrect");
            return ESP_FAIL;
        }
        if(!Checksum(position,buffer))
        {
            ESP_LOGE(__FUNCTION__,"Checksum failed!");
            return ESP_FAIL;
        }
        float v_param = ((uint32_t)buffer[position + 2]  <<16) + ((uint32_t)buffer[position + 3] <<8) + buffer[position + 4];
        float v_data = ((uint32_t)buffer[position + 5]  <<16) + ((uint32_t)buffer[position + 6] <<8) + buffer[position + 7];
        float i_param = ((uint32_t)buffer[position + 8]  <<16) + ((uint32_t)buffer[position + 9] <<8) + buffer[position + 10];
        float i_data = ((uint32_t)buffer[position + 11]  <<16) + ((uint32_t)buffer[position + 12] <<8) + buffer[position + 13];
        float p_param = ((uint32_t)buffer[position + 14]  <<16) + ((uint32_t)buffer[position + 15] <<8) + buffer[position + 16];
        float p_data =  ((uint32_t)buffer[position + 17]  <<16) + ((uint32_t)buffer[position + 18] <<8) + buffer[position + 19];
        meter->Voltage = v_param/v_data * VOLTAGE_COEF;
        meter->Current = i_param/i_data * CURRENT_COEF;
        meter->Power_active = p_param/p_data * VOLTAGE_COEF * CURRENT_COEF;
        meter->Pf = 0.0;
        meter->Power_appa = 0.0;
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