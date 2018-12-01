#include "config.h"
#include "platform.h"
#include "init.h"
#include "canlib/dspic33epxxxgp50x/dspic33epxxxgp50x_can.h"

//required for delay functions
#include <libpic30.h>

void can_callback_function(const can_msg_t *message) {
    //do nothing... for now
}

int main() {
    //initialize the pins first so we can use the LEDs to tell us if init fails
    init_pins();

    //turn on LED 1 (the red one).
    LED_1_ON();

    //wait 10ms
    __delay32(10 * (FCY / 1000));
    //Continue with initialization
    init_oscillator();
    init_peripherals();

    can_timing_t timing;
    timing.brp = 0x3f;
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
