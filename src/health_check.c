#include "canlib.h"
#include "main.h"

#include "health_check.h"
#include "platform.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

#define BATT_CURR_SCALAR_TIMES                                                                     \
    2 // ADC / 4096 * 2.8V / 100V/V / 0.03 Ohm * 1000mA/A = ADC * 0.227 = ADC * 2 / 9
#define BATT_CURR_SCALAR_DIV 9
#define BATTERY_CURRENT_THRESHOLD 400 // Should be even less when running without OV5640

#define BATT_VOLT_SCALAR_TIMES 17 // ADC / 4096 * 2.8V * 5 * 1000 mV/V = ADC * 3.417
#define BATT_VOLT_SCALAR_DIV 5
#define BATT_VOLT_LOW_THRESHOLD 8000
#define BATT_VOLT_HIGH_THRESHOLD 13000 // ADC will saturate by then

uint32_t health_check(void) {
    uint32_t status_msg_general_status = 0;
    can_msg_t msg;

    // 12V Current Check
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 500);
    uint16_t adcval = HAL_ADC_GetValue(&hadc1); // 12-bit ADC
    uint16_t battery_current_mA = adcval * BATT_CURR_SCALAR_TIMES / BATT_CURR_SCALAR_DIV;

    if (battery_current_mA > BATTERY_CURRENT_THRESHOLD) {
        status_msg_general_status |= (1 << E_12V_OVER_CURRENT_OFFSET);
    }

    build_analog_data_msg(PRIO_LOW, millis(), SENSOR_12V_CURR, battery_current_mA, &msg);
    can_send(&msg);

    // 12V Voltage Check
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, 500);
    adcval = HAL_ADC_GetValue(&hadc2); // 12-bit ADC
    uint16_t battery_voltage_mV = adcval * BATT_VOLT_SCALAR_TIMES / BATT_VOLT_SCALAR_DIV;

    if (battery_voltage_mV > BATT_VOLT_HIGH_THRESHOLD) {
        status_msg_general_status |= (1 << E_12V_OVER_VOLTAGE_OFFSET);
    }

    if (battery_voltage_mV < BATT_VOLT_LOW_THRESHOLD) {
        status_msg_general_status |= (1 << E_12V_UNDER_VOLTAGE_OFFSET);
    }

    build_analog_data_msg(PRIO_LOW, millis(), SENSOR_12V_VOLT, battery_voltage_mV, &msg);
    can_send(&msg);

    return status_msg_general_status;
}
