
#include "config.h"
#include "platform.h"
#include "init.h"

//required for delay functions
#include <libpic30.h>

//25 MegaInstructions per second
#define FCY 25000000

int main() {
    //initialize the pins first so we can use the LEDs to tell us if init fails
    init_pins();

    //turn on LED 1 (the red one).
    LED_1_ON();

    //Continue with initialization
    init_oscillator();

    //turn on the blue LED to show that initialization has succeeded
    LED_1_OFF();
    LED_2_ON();

    //wait for 5 seconds
    __delay32(5 * FCY);

    while(1) {
        LED_1_ON();
        LED_2_OFF();
        __delay32(FCY / 2);
        LED_1_OFF();
        LED_2_ON();
        __delay32(FCY / 2);
    }
}