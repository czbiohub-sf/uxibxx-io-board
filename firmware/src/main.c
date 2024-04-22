#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "cmdproc.h"


void hwInit(void) {
	clock_prescale_set(clock_div_1);
	}

void mstick__tick(uint16_t *tickCounter) {
	statusleds__onMsTick(tickCounter);
	}

void triggerWatchdogReset(void) {
	wdt_enable(WDTO_2S);
	cli();
	statusleds__setHbtLed(1);
	_delay_ms(1000);
	usbcdc__detach();
	statusleds__setUsbLed(0);
	while(1) {}
	}

void main(void) {
	cmdproc_command_t command;

	hwInit();
	statusleds__init();
	cmdproc__init();
	mstick__init();
	usbcdc__init();
	sei();

	while(1) {
		usbcdc__task();
		statusleds__task();

		if(cmdproc__hasCommandWaiting()) {
			statusleds__winkUsbLed();
			cmdproc__getCommand(&command);
			if(!strcmp(command.mnem, "DFU"))
				triggerWatchdogReset();

			usbcdc__sendStringNoFlush("command is ");
			usbcdc__sendStringNoFlush(command.mnem);
			usbcdc__sendStringNoFlush(" and first larg is ");
			char asdf[64];
			itoa(command.leftArgs[0].uintVal, asdf, 10);
			usbcdc__sendStringNoFlush(asdf);
			usbcdc__sendStringNoFlush(" and cmdtype is ");
			itoa(command.cmdType, asdf, 10);
			usbcdc__sendStringNoFlush(asdf);
			usbcdc__sendStringNoFlush(" and parseError is ");
			itoa(command.parseError, asdf, 10);
			usbcdc__sendStringNoFlush(asdf);
			usbcdc__sendString("\r\n");
			}
		while(usbcdc__hasInputWaiting()) {
			int16_t ch = usbcdc__getNextInputChar();
			if(ch >= 0)
				cmdproc__processIncomingChar((uint8_t)ch);
			}
		};
	}
