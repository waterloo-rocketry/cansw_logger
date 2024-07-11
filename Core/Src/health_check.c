#include "health_check.h"
#include "can_syslog.h"
#include "canlib.h"
#include "error.h"
#include "message_types.h"
#include "platform.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

#define BATT_CURR_SCALAR 5 // ADC / 4096 * 2.5V / 100V/V / 30mOhm * 1000mA/A = ADC / 4.9
#define BATTERY_CURRENT_THRESHOLD 400 // Should be even less when running without OV5640

#define BATT_VOLT_SCALAR 328 // ADC / 4096 * 2.5V * 5 = ADC / 327.68
#define BATT_VOLT_LOW_THRESHOLD 8000
#define BATT_VOLT_HIGH_THRESHOLD 13000 // ADC will saturate by then

bool check_bus_current_error(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 1000);
    uint16_t adcval = HAL_ADC_GetValue(&hadc1); // 12-bit ADC
    uint16_t battery_current_mA = adcval / BATT_CURR_SCALAR;

    if (battery_current_mA > BATTERY_CURRENT_THRESHOLD) {
        uint32_t timestamp = millis();
        uint8_t curr_data[2] = {0};
        curr_data[0] = (battery_current_mA >> 8) & 0xff;
        curr_data[1] = (battery_current_mA >> 0) & 0xff;

        can_msg_t error_msg;
        build_board_stat_msg(timestamp, E_BATT_OVER_CURRENT, curr_data, 2, &error_msg);
        can_send(&error_msg);

        return true;
    }

    return false;
}

bool check_bus_voltage_error(void) {
    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, 1000);
    uint16_t adcval = HAL_ADC_GetValue(&hadc2); // 12-bit ADC
    uint16_t battery_voltage_mV = adcval / BATT_VOLT_SCALAR;

    if (battery_voltage_mV > BATT_VOLT_HIGH_THRESHOLD) {
        uint32_t timestamp = millis();
        uint8_t volt_data[2] = {0};
        volt_data[0] = (battery_voltage_mV >> 8) & 0xff;
        volt_data[1] = (battery_voltage_mV >> 0) & 0xff;

        can_msg_t error_msg;
        build_board_stat_msg(timestamp, E_BATT_OVER_VOLTAGE, volt_data, 2, &error_msg);
        can_send(&error_msg);

        return true;
    }

    if (battery_voltage_mV < BATT_VOLT_LOW_THRESHOLD) {
        uint32_t timestamp = millis();
        uint8_t volt_data[2] = {0};
        volt_data[0] = (battery_voltage_mV >> 8) & 0xff;
        volt_data[1] = (battery_voltage_mV >> 0) & 0xff;

        can_msg_t error_msg;
        build_board_stat_msg(timestamp, E_BATT_UNDER_VOLTAGE, volt_data, 2, &error_msg);
        can_send(&error_msg);

        return true;
    }

    return false;
}
