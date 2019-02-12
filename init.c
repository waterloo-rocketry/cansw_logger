#include "init.h"
#include "sd.h"
#include "can_syslog.h"
#include <xc.h>
#include <stdbool.h>

//Set up pin registers
void init_pins()
{
    //LEDs off at startup
    LATBbits.LATB13 = 0;
    LATBbits.LATB14 = 0;

    //set LEDs as outputs
    TRISBbits.TRISB13 = 1;
    TRISBbits.TRISB14 = 1;

    //CAN stuff
    TRISBbits.TRISB11 = 0; //set CANTX as output
    RPINR26bits.C1RXR = 0b0101010; //set CAN input to pin RP42/RB10
    RPOR4bits.RP43R = 0b1110; //set CAN output to pin RP43/RB11
}

//Get running off of the external oscillator.
//Currently set to make Fosc 50MHz, which means 25M instructions per second.
void init_oscillator()
{
    //divide input frequency (16M) by 4 to get 4M
    CLKDIVbits.PLLPRE = 0x02;

    //multiply the 4M by 50 to get 200M
    PLLFBDbits.PLLDIV = 0x30;

    //divide the 200M by 4 to get a 50M Fosc
    CLKDIVbits.PLLPOST = 0x01;

    //disable clock dozing, because we currently don't care about power draw
    CLKDIVbits.DOZEN = 0;

    /*
     * This upcoming clock switch thing is annoying. For details on what this
     * assembly does, check out this datasheet
     * http://ww1.microchip.com/downloads/en/DeviceDoc/39700c.pdf
     */

    //to unlock NOSC, need to write 0x9A, 0x78, and NOSC to OSCCONH in three
    //consecutive instructions
    volatile register uint8_t *w = &OSCCONH;
    register uint8_t x = 0x78;
    register uint8_t y = 0x9A;
    //desired NOSC value is lower nibble of z, 3
    register uint8_t z = 0x73;
    //three consecutive writes
    *w = x;
    *w = y;
    *w = z;
    //to perform the clock switch, we need to unlock OSCCONL and write 1 to the
    //lowest bit of OSCCONL. These need to be three consecutive instructions.
    //The two unlock writes are 0x46 and 0x57
    w = &OSCCONL;
    x = 0x46;
    y = 0x57;
    z = 0x01;
    //three consecutive writes
    *w = x;
    *w = y;
    *w = z;

    //wait until we're running off of the external clock
    while (OSCCONbits.COSC != 0x03) {}
}

/*
 * setup timers 2 and 3 to form a 32-bit timer that ticks over every ms
 * configure timer2 interrupt to fire when TMR2 = 25MHz / 1KHz
 */
void init_timers()
{
    //continue while idle
    T2CONbits.TSIDL = 0;
    //disable gated time accumulation
    T2CONbits.TGATE = 0;
    //do no prescaling (drive oscillator with Fp)
    T2CONbits.TCKPS = 0;
    //set as 16 bit timer
    T2CONbits.T32 = 0;
    //use  Fp as input
    T2CONbits.TCS = 0;

    //setup timer2 interrupt to fire when timer2 reaches 25000
    PR2 = 25000;
    //enable timer2 interrupts
    IEC0bits.T2IE = 1;
    //enable timer2
    T2CONbits.TON = 1;
}

void init_peripherals()
{
    init_spi();
    init_sd_card2();
    init_can_syslog();
    init_timers();
}
