#ifndef PLATFORM_H
#define	PLATFORM_H

#define FCY 25000000

#include <xc.h>

#define LED_1_ON()  (TRISBbits.TRISB13 = 0)
#define LED_1_OFF() (TRISBbits.TRISB13 = 1)

#define LED_2_ON()  (TRISBbits.TRISB14 = 0)
#define LED_2_OFF() (TRISBbits.TRISB14 = 1)

uint32_t millis(void);

#endif	/* PLATFORM_H */

