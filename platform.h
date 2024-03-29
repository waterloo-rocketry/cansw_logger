#ifndef PLATFORM_H
#define	PLATFORM_H

#define FCY 48000000

#include <xc.h>

// blue LED
#define LED_1_ON()  (TRISBbits.TRISB13 = 0)
#define LED_1_OFF() (TRISBbits.TRISB13 = 1)

// white LED
#define LED_2_ON()  (TRISBbits.TRISB14 = 0)
#define LED_2_OFF() (TRISBbits.TRISB14 = 1)

//number of milliseconds since bootup
uint32_t millis(void);
//number of microseconds since bootup
uint32_t micros(void);

#endif	/* PLATFORM_H */

