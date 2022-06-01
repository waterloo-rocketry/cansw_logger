#include "health_check.h"
#include "error.h"
#include "adc1.h"
#include "platform.h"
#include "dspic33epxxxgp50x_can.h"
#include "can_syslog.h"
#include "can_common.h"
#include "message_types.h"
#include "can_tx_buffer.h"

bool check_bus_current_error(void) {
    uint16_t battery_current_mA = (uint16_t)ADC1_GetSingleConversion(channel_CURR_AMP)/BATT_CURR_SCALAR;
    
    if (battery_current_mA > BATTERY_CURRENT_THRESHOLD) {
        uint32_t timestamp = millis();
        uint8_t curr_data[2] = {0};
        curr_data[0] = (battery_current_mA >> 8) & 0xff;
        curr_data[1] = (battery_current_mA >> 0) & 0xff;
        
        can_msg_t error_msg;
        build_board_stat_msg(timestamp, E_BUS_OVER_CURRENT, curr_data, 2, &error_msg);
        txb_enqueue(&error_msg);
        
        return true;
    }
    
    return false;
}