#include <stdbool.h>
#include <stdint.h>

#include "main.h"

#include "canlib.h"

#include "platform.h"
#include "sd.h"

extern FDCAN_HandleTypeDef hfdcan1;

#define MAX_BUS_DEAD_TIME_ms 1000

static uint8_t logger_off = 0;
volatile bool seen_can_message = false;

void can_callback_function(const can_msg_t *message, uint32_t) {
    seen_can_message = true;

    // handle a "LED_ON" or "LED_OFF" message
    //  Declare this outside of switch statement to prevent errors
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
    // Stops logging after BUS_DOWN_WARNING
    if (logger_off < 1) {
        handle_can_interrupt(message);
    }
}

void fwmain(void) {
    init_can_syslog();
    init_fs();
    can_init_stm(&hfdcan1, can_callback_function);

    bool led_on = false;
    uint32_t last_blink_time = 0;

    uint32_t last_board_status_msg = 0;
    uint32_t last_message_time = 0;

    for (;;) {
        // lrWdt();

        can_syslog_heartbeat();

        if (seen_can_message) {
            seen_can_message = false;
            last_message_time = millis();
        }

        if (millis() - last_message_time > MAX_BUS_DEAD_TIME_ms) {
            // We've got too long without seeing a valid CAN message (including one of ours)
            NVIC_SystemReset();
        }

        if (millis() - last_blink_time > 500) {
            led_on = !led_on;
            if (led_on) {
                LED_GREEN_ON();
            } else {
                LED_GREEN_OFF();
            }
            last_blink_time = millis();
        }

        // give status update
        if (millis() - last_board_status_msg > 500) {
            last_board_status_msg = millis();
            health_check();
        }
    }

    return;
}
