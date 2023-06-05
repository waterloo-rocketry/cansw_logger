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
#include "timing_util.h"
#include "health_check.h"
#include "adc1.h"
#include <string.h>
#include <libpic30.h>
#include <stdbool.h>

#define MAX_BUS_DEAD_TIME_ms 1000

static uint8_t logger_off = 0;
volatile bool seen_can_message = false;

void can_callback_function(const can_msg_t *message)
{
    seen_can_message = true;
    int dest_id = get_reset_board_id(message);

    //handle a "LED_ON" or "LED_OFF" message
    // Declare this outside of switch statement to prevent errors
    int cmd_type = -1;
    switch (get_message_type(message)) {
        case MSG_GENERAL_CMD:
            cmd_type = get_general_cmd_type(message);
            if (cmd_type == BUS_DOWN_WARNING) {
                logger_off = 40;
            }
            break;
        case MSG_LEDS_ON:
            LED_1_ON();
            LED_2_ON();
            break;
        case MSG_LEDS_OFF:
            LED_1_OFF();
            LED_2_OFF();
            break;
        case MSG_RESET_CMD:
            if(dest_id == BOARD_UNIQUE_ID || dest_id == 0 ) {
                __asm__ volatile ("reset");
            }
            break;
        default:
            break;
    }
    // Stops logging after BUS_DOWN_WARNING
    if(logger_off < 1)
        handle_can_interrupt(message);
}

static uint8_t txb_pool[100];

int main()
{
    //initialize the pins first so we can use the LEDs to tell us if init fails
    init_pins();

    //turn on LED 1 (the blue one)
    LED_1_ON();

    //initialize the oscillator so we're running faster
    init_oscillator();
    init_timers();
    //initialize spi, SD card, and CAN syslog
    init_peripherals();
    txb_init(txb_pool, sizeof(txb_pool), can_send, can_send_rdy);

    //initilize ADC
    ADC1_Initialize();

    //turn off blue LED, since we're done initializing
    LED_1_OFF();

    //timing parameters that cause a bit time of 24us
    /* FCAN is 32MHz,
     * bit time is 5+5+1+1 = 12 time quanta
     * bit time is 12 * (BRP + 1) * 2 / 32= 24
     * so BRP + 1 = 32
     */
    can_timing_t timing;
    can_generate_timing_params(FCY, &timing);
    init_can(&timing, can_callback_function, false);

    //turn on the white LED to show that initialization has succeeded
    LED_2_ON();

    uint32_t last_on_time = 0;
    uint32_t last_board_status_msg = 0;
    uint32_t last_message_time = 0;
    while (1) {
        // clear watchdog timer
        ClrWdt();
        
        can_syslog_heartbeat();
        
        if (seen_can_message) {
            seen_can_message = false;
            last_message_time = millis();
        }
        
        if (millis() - last_message_time > MAX_BUS_DEAD_TIME_ms) {
            // We've got too long without seeing a valid CAN message (including one of ours)
            __asm__ volatile ("reset");
        }
        

        //blink blue LED at 1/3 Hz, duty cycle of 1/12
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
            bool status_ok = true; //t
            status_ok = status_ok & !check_bus_current_error();
            // for now just always pretend everything is ok
            if (any_errors()) {
                uint8_t e = (uint8_t) get_last_error();
                build_board_stat_msg(millis(), E_LOGGING, &e, 1, &board_stat_msg);
                txb_enqueue(&board_stat_msg);
            } else if (status_ok) {
                build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &board_stat_msg);
                txb_enqueue(&board_stat_msg);
            } else {
                //Error message already sent by check_bus_current_error
            }
            
            if(logger_off > 0)
                logger_off--;
            
            last_board_status_msg = millis();
        }

        txb_heartbeat();
    }
}
