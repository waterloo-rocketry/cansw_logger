#include "config.h"
#include "platform.h"
#include "init.h"
#include "canlib/dspic33epxxxgp50x/dspic33epxxxgp50x_can.h"
#include "can_syslog.h"
#include "sd.h"

//required for delay functions
#include <libpic30.h>

void can_callback_function(const can_msg_t *message) {
    //handle a "LED_ON" or "LED_OFF" message
    if((message->sid & 0x7E0) == 0x7E0) {
        LED_1_ON();
        LED_2_ON();
    } else if ((message->sid & 0x7E0) == 0x7C0) {
        LED_1_OFF();
        LED_2_OFF();
    }
}

int main() {
    //initialize the pins first so we can use the LEDs to tell us if init fails
    init_pins();

    //turn on LED 1 (the red one).
    LED_1_ON();

    //wait 20ms. SD card recommends 10, this is just to be safe
    __delay32(20 * (FCY / 1000));
    //Continue with initialization
    init_oscillator();
    init_peripherals();
    LED_2_ON();

    //log three CAN messages to SD card
    can_msg_t msg1, msg2, msg3;
    msg1.sid = 0x7EF;
    msg1.data[0] = 0;
    msg1.data[1] = 1;
    msg1.data[2] = 2;
    msg1.data_len = 3;

    msg2.sid = 0x110;
    msg2.data[0] = 4;
    msg2.data[1] = 5;
    msg2.data[2] = 6;
    msg2.data[3] = 7;
    msg2.data[4] = 8;
    msg2.data[5] = 9;
    msg2.data_len = 6;

    msg3.sid = 0x453;
    msg3.data[0] = 10;
    msg3.data[1] = 11;
    msg3.data[2] = 12;
    msg3.data[3] = 13;
    msg3.data[4] = 14;
    msg3.data[5] = 15;
    msg3.data[6] = 16;
    msg3.data[7] = 107;
    msg3.data_len = 8;

    handle_can_interrupt(&msg1);
    handle_can_interrupt(&msg2);
    handle_can_interrupt(&msg3);

    LED_2_OFF();

    //force those messages to flush
    force_log_everything();

    LED_1_OFF();

    //timing parameters that cause a bit time of 24us
    can_timing_t timing;
    timing.brp = 0x2A;
    timing.sjw = 3;
    timing.btlmode = 1;
    timing.prseg = 3;
    timing.sam = 0;
    timing.seg1ph = 0;
    timing.seg2ph = 0;
    init_can(&timing, can_callback_function, false);

    //turn on the blue LED to show that initialization has succeeded
    LED_1_OFF();
    LED_2_OFF();

    while(1) {
    }
}
