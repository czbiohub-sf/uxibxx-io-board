#include <stdint.h>
#include <avr/interrupt.h>

#include "mstick.h"


static volatile uint16_t tickCounter;


void mstick__init(void) {
	tickCounter = 0;
	OCR0A = (F_CPU / 64) / 1000;
	TCCR0A = _BV(WGM01); //count up to OCR0A
	TIMSK0 = _BV(OCIE0A); //enable interrupt on OCR0A match
	TCCR0B = _BV(CS01) | _BV(CS00); //start Timer0 at 1/64
	}

ISR(TIMER0_COMPA_vect) {
	++tickCounter;
	mstick__tickEvent(&tickCounter);
	}
