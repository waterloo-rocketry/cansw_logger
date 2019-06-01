#include "config.h"
#include "platform.h"
#include "init.h"
#include "dspic33epxxxgp50x_can.h"
#include "can_syslog.h"
#include "can_common.h"
#include "message_types.h"
#include "can_tx_buffer.h"
#include "sd.h"
#include "error.h"

#include <string.h>
#include <libpic30.h>


void can_callback_function(const can_msg_t *message)
{
    //handle a "LED_ON" or "LED_OFF" message
    switch (get_message_type(message)) {
        case MSG_LEDS_ON:
            //LED_1_ON();
            LED_2_ON();
            break;
        case MSG_LEDS_OFF:
            //LED_1_OFF();
            LED_2_OFF();
            break;
        default:
            break;
    }
    handle_can_interrupt(message);
}

static uint8_t txb_pool[100];

int main()
{
    //initialize the pins first so we can use the LEDs to tell us if init fails
    init_pins();

    //turn on LED 1 (the red one)
    LED_1_ON();

    //initialize the oscillator so we're running faster
    init_oscillator();
    init_timers();
    //initialize spi, SD card, and CAN syslog
    init_peripherals();
    txb_init(txb_pool, sizeof(txb_pool), can_send, can_send_rdy);


    //turn off red LED, since we're done initializing
    LED_1_OFF();

    //timing parameters that cause a bit time of 24us
    /* FCAN is 32MHz,
     * bit time is 5+5+1+1 = 12 time quanta
     * bit time is 12 * (BRP + 1) * 2 / 32= 24
     * so BRP + 1 = 32
     */
    can_timing_t timing;
    timing.brp = 18;
    timing.sjw = 3;
    timing.btlmode = 1;
    timing.prseg = 0;
    timing.sam = 0;
    timing.seg1ph = 4;
    timing.seg2ph = 4;
    init_can(&timing, can_callback_function, false);

    //turn on the blue LED to show that initialization has succeeded
    LED_1_OFF();
    LED_2_OFF();

    uint32_t last_on_time = 0;
    uint32_t last_board_status_msg = 0;
    while (1) {
        can_syslog_heartbeat();

        //blink red LED at 1/3 Hz, duty cycle of 1/12
        if (millis() - last_on_time < 250) {
            LED_1_ON();
        } else if (millis() - last_on_time < 3000) {
            LED_1_OFF();
        } else {
            last_on_time = millis();
        }

        //give status update
        if (millis() - last_board_status_msg > 500) {
            can_msg_t board_stat_msg;
            // for now just always pretend everything is ok
            if (any_errors()) {
                uint8_t e = (uint8_t) get_last_error();
                build_board_stat_msg(millis(), E_LOGGING, &e, 1, &board_stat_msg);
            } else {
                build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &board_stat_msg);
            }
            txb_enqueue(&board_stat_msg);
        }

        txb_heartbeat();
    }
}
