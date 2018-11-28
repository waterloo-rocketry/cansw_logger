#include "sd.h"
#include <xc.h>
#include <libpic30.h>
#include "platform.h"

#define cs_high() (LATBbits.LATB9 = 1)
#define cs_low()  (LATBbits.LATB9 = 0)
#define FCY 25000000

static void spi2_send(uint8_t data) {
    __delay32(FCY / 1000);
    while(SPI2STATbits.SPITBF) {}
    SPI2BUF = data;
    while(!SPI2STATbits.SPIRBF) {}
    uint8_t temp = SPI2BUF;
}

static uint8_t spi2_read(void) {
    __delay32(FCY / 1000);
    while(SPI2STATbits.SPITBF) {};
    SPI2BUF = 0xFF;
    while(!SPI2STATbits.SPIRBF) {};
    uint8_t temp = SPI2BUF;
    //wait until SPIBEC is 0
    return temp;
}


uint8_t init_sd_card2() {
    //based on a tutorial, set CS and MOSI high, and toggle SCK 74 times
    cs_high();
    int i;
    for(i = 0; i < 10; ++i) {
        spi2_send(0xFF);
    }
    for(i = 0; i < 1000; ++i) ;
    //after that, set cs to 0n and send
    // 0b01000000 00000000 00000000 00000000 00000000 10010101
    cs_low();
    spi2_send(0x40);
    spi2_send(0x00);
    spi2_send(0x00);
    spi2_send(0x00);
    spi2_send(0x00);
    spi2_send(0x95);

    //read a byte from SPI. It should be 0x01
    uint8_t status = spi2_read();

    for(i = 0; i < 10; ++i) {
        if(status == 0x01) {
            break;
        }
        status = spi2_read();
    }

    cs_high();
    return status == 0x01;
}

void init_spi() {
    //enable spi module 2 as master mode
    SPI2CON1bits.DISSCK = 0; //enable sck
    SPI2CON1bits.DISSDO = 0; //enable SDO
    SPI2CON1bits.MODE16 = 0; //8 bit things
    SPI2CON1bits.SMP    = 0; //sample at middle of data time
    SPI2CON1bits.CKE    = 0; //switch output on rising edge of SCK
    SPI2CON1bits.SSEN   = 0; //we are not in slave mode, leave CS GPIO
    SPI2CON1bits.CKP    = 1; //idle clock level high.
    SPI2CON1bits.MSTEN  = 1; //use master mode
    SPI2CON1bits.SPRE   = 0; //secondary prescale 8:1
    SPI2CON1bits.PPRE   = 0; //primary prescale 8:1
    SPI2CON2bits.FRMEN  = 0; //don't use framed mode
    SPI2CON2bits.SPIBEN = 0; //use standard mode, not enhanced mode

    //set SCK output to RP39, and input to RP32 (both must be set)
    RPOR2bits.RP39R = 0x09;
    RPINR22bits.SCK2R = 0x27;
    //set MOSI output to RP40
    RPOR3bits.RP40R = 0x08;
    //set MISO input to RP38 (RB6)
    RPINR22bits.SDI2R = 0x26;
    TRISBbits.TRISB6 = 1;
    //set CS as GPIO output on RB9. Start high.
    TRISBbits.TRISB9 = 0;
    LATBbits.LATB9 = 1;

    //enable spi module 1
    SPI2STATbits.SPIEN = 1;
}
