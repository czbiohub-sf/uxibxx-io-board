#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>


void hwInit(void) {
	clock_prescale_set(clock_div_1);
	}

void mstick__tick(uint16_t *tickCounter) {
	statusleds__onMsTick(tickCounter);
	}

void main(void) {
	hwInit();
	statusleds__init();
	mstick__init();
	usbcdc__init();
	sei();

	while(1) {
		usbcdc__task();
		statusleds__task();
		};
	}
