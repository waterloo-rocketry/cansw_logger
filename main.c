#include "config.h"
#include "platform.h"
#include "init.h"

//required for delay functions
#include <libpic30.h>

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

    //turn on the blue LED to show that initialization has succeeded
    LED_1_OFF();
    LED_2_ON();

    //wait for 5 seconds, then turn the LED off
    __delay32(5 * FCY);
    LED_2_OFF();

    while(1) {
    }
}
