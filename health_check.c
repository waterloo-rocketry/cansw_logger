#include "health_check.h"
#include "error.h"
#include "constants.h"
#include "mcc_generated_files/adc1.h"

void health_check_current_error(void) {
    uint16_t battery_current = (uint16_t)ADC1_GetSingleConversion(channel_CURR_AMP)*BATT_CURR_SCALAR;
    
    if (battery_current > BATTERY_CURRENT_THRESHOLD) {
        error(E_CURRENT_DRAW);
    }
    
}