/*
 * File:   main.c
 * Author: lammersdorf
 *
 * Created on January 18, 2025, 5:40 PM
 */
/*               |---\/---|
 *       RA2 <-> |        | <-> RA1
 *       RA3 <-> |        | <-> RA0
 * RA4/T0CK1 <-> |        | <-  OSC1/CLKIN
 *     !MCLR  -> |        | ->  OSC2/CLKOUT
 *       VSS  -> |        | <-  VDD
 *   RB0/INT <-> |        | <-> RB7
 *       RB1 <-> |        | <-> RB6
 *       RB2 <-> |        | <-> RB5
 *       RB3 <-> |        | <-> RB4
 *               |--------|
 * PIC beim Einschalten: alle Ports sind Inputs 1 = Eingang, 0 = Ausgang
 */
#include <xc.h>
#include <pic16f84.h>

#define _XTAL_FREQ    4000000

#pragma config WDTE = OFF
#pragma config FOSC = XT
#pragma config PWRTE = OFF 
#pragma config CP = OFF

#define SET_OUTPUT    0
#define SET_INPUT     1
#define SET_HIGH      1
#define SET_LOW       0
#define IS_HIGH       1
#define IS_LOW        0

#define NOTAUS_INT PORTBbits.RB0 
#define SCL        PORTBbits.RB1
#define SDA        PORTBbits.RB2
#define RELAY_1    PORTBbits.RB3
#define RELAY_2    PORTBbits.RB4
#define TASTE_1    PORTAbits.RA0
#define TASTE_2    PORTAbits.RA1

void clearAllRelay();
void sendDat(uint8_t dat);
uint8_t isrInfo = 0;

/*
 * Send Relay-Nr.
 * SDA und SCL sind high, SCL 1/2 low set SDA, SCL 1/2 high, Empf‰nger lieﬂt
 * Start, 8 bits, Stop
 * 
 */
void sendDat(uint8_t dat) {
    int maske = 0b00000001;
    
    for(uint8_t i = 0; i < 8; ++i) {
        SCL = SET_LOW;
        if (maske & dat)
            SDA = SET_HIGH;
        else
            SDA = SET_LOW;
        maske <<= 1;
        __delay_us(90);
        SCL = SET_HIGH;
        __delay_us(100);
    }
}

void clearAllRelay() {
    RELAY_1 = SET_LOW;
    RELAY_2 = SET_LOW;
}

int main(void) {
    TRISBbits.TRISB0 = SET_INPUT;   //INT f¸r Notaus
    TRISBbits.TRISB1 = SET_OUTPUT;  //I2C-Clock SCL
    TRISBbits.TRISB2 = SET_OUTPUT;  //I2C-Data  SDA Send
    TRISBbits.TRISB3 = SET_OUTPUT;  //Relay_1
    TRISBbits.TRISB4 = SET_OUTPUT;  //Relay_2
    TRISAbits.TRISA0 = SET_INPUT;   //Taste_1
    TRISAbits.TRISA1 = SET_INPUT;   //Taste_2
    
    SCL = SET_HIGH;
    SDA = SET_HIGH;
    
    RELAY_1 = SET_LOW;
    RELAY_2 = SET_LOW;
    
    INTCONbits.GIE = SET_HIGH;  //Interrups sind mˆglich
    INTCONbits.INTE = SET_HIGH; //Interrupt an INT ist mˆglich
    
    while(1) {
        if (TASTE_1 == IS_LOW) {
            clearAllRelay();
            RELAY_1 = SET_HIGH;
            sendDat(1);
            while(TASTE_1 == IS_LOW) ;
        }
        if (TASTE_2 == IS_LOW) {
            clearAllRelay();
            RELAY_2 = SET_HIGH;
            sendDat(2);
            while(TASTE_2 == IS_LOW) ;
        }
        if (isrInfo) {
            sendDat(0);  //0: all off
            isrInfo = 0;
        }
    }
    return 0;
}

 void __interrupt() ISR(void) {
    if (INTCONbits.INTF) {
        if (!NOTAUS_INT) {
            RELAY_1 = SET_LOW;
            RELAY_2 = SET_LOW;
            isrInfo = 1;
        }
        INTCONbits.INTF = 0;
    }
}
