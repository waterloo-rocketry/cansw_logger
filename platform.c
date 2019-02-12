#include "platform.h"
#include <stdint.h>

static uint32_t millis_counter = 0;
uint32_t millis()
{
    return millis_counter;
}

void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt(void)
{
    TMR2 = 0;
    millis_counter++;
    IFS0bits.T2IF = 0;
    return;
}