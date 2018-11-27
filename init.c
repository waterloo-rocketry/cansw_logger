#include "init.h"
#include <xc.h>

//Set up pin registers
void init_pins() {
    //LEDs off at startup
    LATBbits.LATB13 = 0;
    LATBbits.LATB14 = 0;

    //set LEDs as outputs
    TRISBbits.TRISB13 = 1;
    TRISBbits.TRISB14 = 1;

}

