/*
 *   ATMEL ATmega 328-P-DS- 48A-PA- 88A-PA- 168A-PA-Belegung
 *
 *              	              +-\/-+
 * 	PCINT14			RESET   PC6	 1|    |28	PC5 ADC5	 SCL	 PCINT13
 * 	PCINT16			RXD 	PD0	 2|    |27	PC4 ADC4	 SDA	 PCINT12
 * 	PCINT17			TXD 	PD1	 3|    |26	PC3 ADC3			 PCINT11
 * 	PCINT18			INT0 	PD2	 4|    |25	PC2 ADC2			 PCINT10
 * 	PCINT19	OC2B	INT1 	PD3	 5|    |24	PC1 ADC1			 PCINT9
 * 	PCINT20	XCK		T0 		PD4	 6|    |23	PC0 ADC0			 PCINT8
 * 				VCC				 7|    |22			 GND
 * 				GND				 8|    |21			 AREF
 * 	PCINT6	XTAL1	TOSC1   PB6	 9|    |20			 AVCC
 * 	PCINT7	XTAL2	TOSC2   PB7	10|    |19	PB5 SCK				 PCINT5
 * 	PCINT21	OC0B	T1 		PD5	11|    |18	PB4 MISO			 PCINT4
 * 	PCINT22	OC0A	AIN0 	PD6	12|    |17	PB3 MOSI	    OC2A PCINT3
 * 	PCINT23			AIN1 	PD7	13|    |16	PB2 SS		    OC1B PCINT2
 * 	PCINT0	CLKO	ICP1 	PB0	14|    |15	PB1 			OC1A PCINT1
 *              	              +----+
*/

#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "pin.h"

#define STROM_POS    PINDEF(C, 0)   //ADC0
#define STROM_NEG    PINDEF(C, 1)   //ADC1
#define STROM_12V    PINDEF(C, 2)   //ADC2
#define STROM_5V     PINDEF(C, 3)   //ADC3
#define I2C_SCL      PINDEF(C, 4)   //PCINT12
#define I2C_SDA      PINDEF(C, 5)
#define DISPL_CKL    PINDEF(D, 2)
#define DISPL_DATA   PINDEF(D, 3)
#define DISPL_LOAD_1 PINDEF(D, 5)
#define DISPL_LOAD_2 PINDEF(D, 6)
#define DISPL_LOAD_3 PINDEF(D, 7)
#define DISPL_LOAD_4 PINDEF(B, 0)
#define NOT_AUS      PINDEF(B, 1)
#define MAX_AMP      250

enum {
    adc0,
    adc1,
    adc2,
    adc3
};
enum {
    notaus_an,
    notaus_aus
};

volatile uint8_t amp;
volatile uint8_t notaus = 1;

void setup();
void changeADInput();
void sendDatToDisplay(uint8_t *dat, uint8_t displNr);
void getData(uint8_t *dat);


void changeADInput() {
    static uint8_t adInput = 0;
    switch (adInput) {
        case adc0: adInput = adc1; break;
        case adc1: adInput = adc2; break;
        case adc2: adInput = adc3; break;
        case adc3: adInput = adc0; break;
    }
    ADMUX &= 0b11110000 + adInput;
}

void setup() {
    setOutput(NOT_AUS);
    clrPin(NOT_AUS);
    notaus = notaus_aus;
    _delay_ms(10);

    setInput(STROM_POS);
    setInput(STROM_NEG);
    setInput(STROM_12V);
    setInput(STROM_5V);
    setInput(I2C_SCL);
    setInput(I2C_SDA);
    setOutput(DISPL_CKL);
    setOutput(DISPL_DATA);
    setOutput(DISPL_LOAD_1);
    setOutput(DISPL_LOAD_2);
    setOutput(DISPL_LOAD_3);
    setOutput(DISPL_LOAD_4);
    setOutput(NOT_AUS);
    setPin(DISPL_CKL);
    setPin(DISPL_DATA);
    setPin(DISPL_LOAD_1);
    setPin(DISPL_LOAD_2);
    setPin(DISPL_LOAD_3);
    setPin(DISPL_LOAD_4);

    //ADC einrichten
    ADMUX |= (1 << REFS1) | (1 << REFS0); //Auswahl der Referenzspannung TODO
    ADMUX |= (1 << ADLAR);                //Linksbündige Ausgabe         TODO
    ADMUX &= 0b11110000 + adc0;           //
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADIE);  //Freigabe, starten, Interruptfreigabe,

    DIDR0 = 0b00110000;
    ADCSRB = 0;

    notaus = notaus_aus;
    sei();
 }

int main() {
    setup();


	return 0;
}

ISR(ADC_vect) {
    if (ADCH >= MAX_AMP) {
        clrPin(NOT_AUS);
        notaus = notaus_an;
    }
    amp = ADCH;
    changeADInput();
}

ISR(PCINT1_vect) {

}
