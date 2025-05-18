#include "canlib.h"
#include "main.h"

#include "fs.h"
#include "health_check.h"
#include "log.h"
#include "platform.h"

#define BOARD_HEARTBEAT_PERIOD 500

extern FDCAN_HandleTypeDef hfdcan1;

void can_callback_function(const can_msg_t *message, uint32_t) {
    switch (get_message_type(message)) {
        case MSG_LEDS_ON:
            LED_RED_ON();
            LED_GREEN_ON();
            break;
        case MSG_LEDS_OFF:
            LED_RED_OFF();
            LED_GREEN_OFF();
            break;
        case MSG_RESET_CMD:
            if (check_board_need_reset(message)) {
                NVIC_SystemReset();
            }
            break;
        default:
            break;
    }

    log_handle_incoming_message(message);
}

uint32_t last_board_status_msg = 0;
bool green_led_on = false;

void fwmain(void) {
    can_init_stm(&hfdcan1, can_callback_function);
    fs_init();
    log_init();

    for (;;) {
        if (millis() - last_board_status_msg > BOARD_HEARTBEAT_PERIOD) {
            last_board_status_msg = millis();

            can_msg_t msg;
            uint32_t general_error_code = health_check();
            build_general_board_status_msg(PRIO_HIGH, millis(), general_error_code, 0, &msg);
            can_send(&msg);

            if (green_led_on) {
                LED_GREEN_OFF();
                green_led_on = false;
            } else {
                LED_GREEN_ON();
                green_led_on = true;
            }
        }

        log_heartbeat();
    }
}
