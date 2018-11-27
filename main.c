
#include "config.h"
#include "platform.h"
#include "init.h"

//required for delay functions
#include <libpic30.h>

#define FCY 1000000

int main() {
    init_pins();

    while(1) {
        LED_1_ON();
        LED_2_OFF();
        __delay32(4000000);
        LED_1_OFF();
        LED_2_ON();
        __delay32(4000000);
    }
}