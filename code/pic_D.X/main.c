/*
 * File:   main.c
 * Author: gfl
 *
 * Created on 19. Januar 2025, 15:07
 *               |---\/---|
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


#define SCL           PORTBbits.RB1
#define SDA           PORTBbits.RB2
#define DISP_SDCLk    PORTAbits.RA2
#define DISP_DATA     PORTAbits.RA3
#define DISP_LOAD     PORTAbits.RA4


static  const uint8_t zeich[15][6] = {
{0x0E,0x33,0x55,0x79,0x8E},	//'0'
{0x04,0x2C,0x44,0x64,0x8E},
{0x1E,0x21,0x46,0x68,0x9F},
{0x1E,0x21,0x4E,0x61,0x9E},
{0x06,0x2A,0x5F,0x62,0x82},
{0x1F,0x30,0x5E,0x61,0x9E},
{0x06,0x28,0x5E,0x71,0x8E},
{0x1F,0x22,0x44,0x68,0x88},
{0x0E,0x31,0x4E,0x71,0x8E},
{0x0E,0x31,0x4F,0x62,0x8C},
{0x00,0x20,0x40,0x60,0x80}, //space
{0x04,0x2A,0x5F,0x71,0x91}, //'A'
{0x11,0x31,0x51,0x6A,0x84}, //'V'
{0x00,0x2A,0x55,0x71,0x91}, //'m''
{0x00,0x20,0x5F,0x60,0x80}  //'-'
};

uint8_t reseiveDat();
void sendToDisplay(uint8_t erg);
void sendBitsToDisplay(uint8_t z);

char *text[] = {"---------", " 3,0 V", " 5,0 V", " 7,5 V", " 9,0 V", "12,0 V"};

/*
 * Reseive Relay-Nr.
 * SDA und SCL sind high, SCL 1/2 low set SDA, SCL 1/2 high, Empf‰nger lieﬂt
 * Start, 8 bits, Stop
 * 
 */
uint8_t reseiveDat() {
    uint8_t dat = 0;
    uint8_t maske = 1;
    
    for (uint8_t i = 0; i < 8; ++i) {
        while(SCL == IS_LOW)
            ;
        if(SDA == IS_HIGH)
            dat += maske;
        maske <<= 1;
        while(SCL == IS_HIGH)
            ;
    }
    return dat;
}

void sendToDisplay(uint8_t erg) {
    char *txt;
    txt = text[erg];
    const uint8_t *zeichen;
    for (uint8_t i = 0; i < 10; ++i) {  //Zeichen
        switch(txt[i]) {
            case '0': zeichen = zeich[0]; break; 
            case '1': zeichen = zeich[1]; break; 
            case '2': zeichen = zeich[2]; break; 
            case '3': zeichen = zeich[3]; break; 
            case '4': zeichen = zeich[4]; break; 
            case '5': zeichen = zeich[5]; break; 
            case '6': zeichen = zeich[6]; break; 
            case '7': zeichen = zeich[7]; break; 
            case '8': zeichen = zeich[8]; break; 
            case '9': zeichen = zeich[9]; break; 
            case ' ': zeichen = zeich[10]; break;
            case 'A': zeichen = zeich[11]; break; 
            case 'V': zeichen = zeich[12]; break; 
            case 'm': zeichen = zeich[13]; break; 
            case '-': zeichen = zeich[14]; break; 
        }
        
        sendBitsToDisplay(0xB0 + i);
        for (uint8_t j = 0; j < 5; ++j)     //Zeilen des Zeichens
            sendBitsToDisplay(zeichen[j]);    
    }
}

void sendBitsToDisplay(uint8_t z) {    
    uint8_t maske = 1;
    DISP_SDCLk = IS_LOW;
    DISP_LOAD = IS_LOW;

    for (uint8_t k = 0; k < 8; ++k) { //bits der Nachricht
        if (z & maske) DISP_DATA = IS_HIGH;
        else           DISP_DATA = IS_LOW;
        DISP_SDCLk = IS_HIGH;
        maske <<= 1;
        DISP_SDCLk = IS_LOW;
    }
    DISP_LOAD = IS_HIGH;
}

int main(void) {
    uint8_t bitZahl = 0;
    uint8_t dat = 0;
    
    TRISBbits.TRISB1 = SET_INPUT;
    TRISBbits.TRISB2 = SET_INPUT;
    TRISAbits.TRISA1 = SET_OUTPUT;
    TRISAbits.TRISA2 = SET_OUTPUT;
    TRISAbits.TRISA2 = SET_OUTPUT;
    
    DISP_LOAD = IS_HIGH;
    DISP_DATA = IS_LOW;
    DISP_SDCLk = IS_LOW;
    
    __delay_ms(1000);
    
    while(1) {
        while(SCL == IS_HIGH) //warten auf Nachricht
            ;
        dat = reseiveDat();
        sendToDisplay(dat);
    }
    
    return 0;
}
