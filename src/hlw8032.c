
#include "hlw8032.h"

esp_err_t hlw8032_serial_begin(hlw8032_t* hlw8032, uart_port_t UART_number, gpio_num_t RX_pin_number, uint16_t UART_buf_size)
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
    
    err = uart_driver_install(UART_number, UART_buf_size, 0, 0, NULL, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(HLW8032_TAG, "UART driver install failed");
        return err;
    }
    err = uart_param_config(UART_number, &uart_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(HLW8032_TAG, "UART param config failed");
        return err;
    }
    uart_set_pin(UART_number, UART_PIN_NO_CHANGE, RX_pin_number, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK)
    {
        ESP_LOGE(HLW8032_TAG, "UART pin setting failed");
        return err;
    }

    hlw8032->UART_num = UART_number;

    return err;
}

esp_err_t hlw8032_read(hlw8032_t* hlw8032)
{
    uint8_t check_search = 0;
    size_t data_len = 0;
    uart_get_buffered_data_len(hlw8032->UART_num, &data_len);

    while ((int)data_len > 0 && check_search != 0x5A)
    {
        uart_read_bytes(hlw8032->UART_num, &check_search, 1, portMAX_DELAY);
        uart_get_buffered_data_len(hlw8032->UART_num, &data_len);
    }
    if ((int)data_len >= 23 && check_search == 0x5A)
    {
        uint8_t* data = (uint8_t*)hlw8032->buffer;
        data[0] = check_search;
        const int rxBytes = uart_read_bytes(hlw8032->UART_num, data + 1, 23, portMAX_DELAY);
        if (rxBytes > 0)
        {
            if (data[23] != 0x55 && data[23] < 0xF0)
            {
                ESP_LOGW(HLW8032_TAG, "Invalid status REG -> 0x%02X", data[23]);
                uart_flush(hlw8032->UART_num);
                return ESP_FAIL;
            }

            uint8_t checksum = 0;
            for(uint8_t a = 1; a<=21; a++)
            {
                checksum = checksum + data[a];
            }
            if (checksum != data[22])
            {
                ESP_LOGW(HLW8032_TAG, "Checksum failed");
                uart_flush(hlw8032->UART_num);
                hlw8032->buffer[23] = 2;
                return ESP_FAIL;
            }

            hlw8032->VoltagePar = ((uint32_t)data[1]  <<16) + ((uint32_t)data[2] <<8) + data[3]; 
            if (bitRead(data[19], 6) == 1)
            {
                hlw8032->VoltageData = ((uint32_t)data[4]  <<16) + ((uint32_t)data[5] <<8) + data[6];
            }

            hlw8032->CurrentPar = ((uint32_t)data[8]  <<16) + ((uint32_t)data[9] <<8) + data[10];  
            if (bitRead(data[19], 5) == 1)
            {
                hlw8032->CurrentData = ((uint32_t)data[10]  <<16) + ((uint32_t)data[11] <<8) + data[12]; 
            }

            hlw8032->PowerPar = ((uint32_t)data[13]  <<16) + ((uint32_t)data[14] <<8) + data[15];
            if(bitRead(data[19], 4) == 1)
            {
                hlw8032->PowerData = ((uint32_t)data[16]  <<16) + ((uint32_t)data[17] <<8) + data[18];
            }

            hlw8032->PowerCoef = ((uint32_t)data[20] <<8) + data[21];
            if(bitRead(data[19], 7) == 1)
            {
                hlw8032->PowerCoefData++;
            }
        }
        else
        {
        hlw8032->buffer[23] = 1;
        ESP_LOGE(HLW8032_TAG,"  CAE AQUI");
        return ESP_FAIL;
        } 
    }
    else
    {
        hlw8032->buffer[23] = 0;
        ESP_LOGE(HLW8032_TAG,"CAE AQUI2");
        return ESP_FAIL;
    } 
    
    uart_flush(hlw8032->UART_num);
    return ESP_OK;
}

void hlw8032_set_V_coef (hlw8032_t* hlw8032, float newVcoef)
{
    hlw8032->VoltageCoef = newVcoef;
    return;
}

void hlw8032_set_I_coef (hlw8032_t* hlw8032, float newIcoef)
{
    hlw8032->CurrentCoef = newIcoef;
    return;
}

void hlw8032_set_V_coef_from_R (hlw8032_t* hlw8032, float R_live, float R_gnd)
{
    hlw8032->VoltageCoef = R_live/(R_gnd*1000);
    return;
}

void hlw8032_set_I_coef_from_R (hlw8032_t* hlw8032, float R)
{
    hlw8032->CurrentCoef = 1.0f/(R*1000);
    return;
}

float hlw8032_get_V (hlw8032_t* hlw8032)
{
    float V = 0;

    if((float)hlw8032->VoltageData > 0)
    V = ((float)hlw8032->VoltagePar/(float)hlw8032->VoltageData)*hlw8032->VoltageCoef;

    return V;
}

float hlw8032_get_V_analog (hlw8032_t* hlw8032)
{
    float V = 0;

    if((float)hlw8032->VoltageData > 0)
    V = ((float)hlw8032->VoltagePar/(float)hlw8032->VoltageData);

    return V;
}

float hlw8032_get_I (hlw8032_t* hlw8032)
{
    float I = 0;

    if((float)hlw8032->CurrentData > 0)
    I = ((float)hlw8032->CurrentPar/(float)hlw8032->CurrentData)*hlw8032->CurrentCoef;

    return I;
}

float hlw8032_get_I_analog (hlw8032_t* hlw8032)
{
    float I = 0;

    if((float)hlw8032->CurrentData > 0)
    I = ((float)hlw8032->CurrentPar/(float)hlw8032->CurrentData);

    return I;
}

float hlw8032_get_P_active (hlw8032_t* hlw8032)
{
    float P = 0;

    if ((float)hlw8032->PowerData > 0)
    P = ((float)hlw8032->PowerPar/(float)hlw8032->PowerData)*hlw8032->VoltageCoef*hlw8032->CurrentCoef;

    return P;
}

float hlw8032_get_P_apparent (hlw8032_t* hlw8032)
{
    float V = hlw8032_get_V(hlw8032);
    float I = hlw8032_get_I(hlw8032);
    
    float P = V*I;
    return P;
}

float hlw8032_get_P_factor (hlw8032_t* hlw8032)
{
    float Pfactor = 0;

    float P_active = hlw8032_get_P_active(hlw8032);
    float P_apparent = hlw8032_get_P_apparent(hlw8032);

    if (P_apparent > 0)
    Pfactor = P_active/P_apparent;

    return Pfactor;
}

uint32_t hlw8032_get_P_coef_all (hlw8032_t* hlw8032)
{
    uint32_t Pcoef_all = hlw8032->PowerCoefData * hlw8032->PowerCoef;
    return Pcoef_all;
}

float hlw8032_get_kwh (hlw8032_t* hlw8032)
{
    float kwh = 0;
    float P_apparent = hlw8032_get_P_apparent(hlw8032);

    if ((float)hlw8032->PowerPar > 0 && P_apparent > 0)
    {
        float Pcoef_count = (1/(float)hlw8032->PowerPar)*(1/P_apparent)*1000000000*3600;

        if (Pcoef_count > 0)
        kwh = (float)hlw8032_get_P_coef_all(hlw8032)/Pcoef_count;
    }
    

    return kwh;
}