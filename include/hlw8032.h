#ifndef HLW8032_H
#define HLW8032_H

#include "string.h"

#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define HLW8032_TAG "HLW8032"

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

//HLW8032 structure
typedef struct {
    float VoltageCoef;
    float CurrentCoef;
    uint32_t PowerCoef;
    uint32_t PowerCoefData;

    uint32_t VoltagePar;
    uint32_t CurrentPar;
    uint32_t PowerPar;

    uint32_t VoltageData;
    uint32_t CurrentData;
    uint32_t PowerData;

    uint8_t buffer[50];

    uart_port_t UART_num;
} hlw8032_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief Start HLW8032 UART serial communication according to params
* @param[in] hlw8032 HLW8032 structure
* @param[in] UART_number UART port number to be used
* @param[in] RX_pin_number GPIO number for receiving data
* @param[in] UART_buf_size Buffer size in bytes
*/
esp_err_t hlw8032_serial_begin(hlw8032_t* hlw8032, uart_port_t UART_number, gpio_num_t RX_pin_number, uint16_t UART_buf_size);

/**
* @brief Read values from UART to update values in the HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
esp_err_t hlw8032_read(hlw8032_t* hlw8032);

/**
* @brief Set voltage coefficient to be used by the HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
* @param[in] newVcoef New value for voltage coefficient
*/
void hlw8032_set_V_coef (hlw8032_t* hlw8032, float newVcoef);

/**
* @brief Set current coefficient to be used by the HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
* @param[in] newVcoef New value for current coefficient
*/
void hlw8032_set_I_coef (hlw8032_t* hlw8032, float newIcoef);

/**
* @brief Set voltage coefficient to be used by the HLW8032 structure using resistor values
* @param[in] hlw8032 HLW8032 structure
* @param[in] R_live Total resistor value towards live
* @param[in] R_gnd Total resistor value towards ground
*/
void hlw8032_set_V_coef_from_R (hlw8032_t* hlw8032, float R_live, float R_gnd);

/**
* @brief Set current coefficient to be used by the HLW8032 structure using resistor value
* @param[in] hlw8032 HLW8032 structure
* @param[in] R Resistor value
*/
void hlw8032_set_I_coef_from_R (hlw8032_t* hlw8032, float R);

/**
* @brief Get voltage value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
float hlw8032_get_V (hlw8032_t* hlw8032);

/**
* @brief Get raw voltage value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
float hlw8032_get_V_analog (hlw8032_t* hlw8032);

/**
* @brief Get current value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
float hlw8032_get_I (hlw8032_t* hlw8032);

/**
* @brief Get raw current value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
float hlw8032_get_I_analog (hlw8032_t* hlw8032);

/**
* @brief Get active power value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
float hlw8032_get_P_active (hlw8032_t* hlw8032);

/**
* @brief Get apparent power value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
float hlw8032_get_P_apparent (hlw8032_t* hlw8032);

/**
* @brief Get power factor value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
float hlw8032_get_P_factor (hlw8032_t* hlw8032);

/**
* @brief Get power coefficient value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
uint32_t hlw8032_get_P_coef_all (hlw8032_t* hlw8032);

/**
* @brief Get kwh value from HLW8032 structure
* @param[in] hlw8032 HLW8032 structure
*/
float hlw8032_get_kwh (hlw8032_t* hlw8032);

#ifdef __cplusplus
}
#endif

#endif