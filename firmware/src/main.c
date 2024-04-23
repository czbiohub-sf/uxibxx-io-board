#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "cmdproc.h"
#include "gpio.h"
#include "mstick.h"
#include "statusleds.h"
#include "usbcdc.h"


void hwInit(void) {
	clock_prescale_set(clock_div_1);
	}

void triggerWatchdogReset(void) {
	wdt_enable(WDTO_2S);
	_delay_ms(1000);
	cli();
	usbcdc__detach();
	statusleds__setUsbLed(0);
	while(1) {}
	}

void handleCommand(void) {
	cmdproc_command_t command;
	char msgOutBuf[32];
	int cmdResult;
	int abort = 0;

	if(cmdproc__hasCommandWaiting()) {
		statusleds__winkUsbLed();
		cmdproc__getCommand(&command);
		if(command.parseError)
			usbcdc__sendString("ERROR:FMT\r\n");
		if(!strcmp(command.mnem, "DFU")) {
			usbcdc__sendString("OK\r\n");
			triggerWatchdogReset();
			}
		else if(!strcmp(command.mnem, "TLS")) {
			usbcdc__sendStringNoFlush("TLS=");
			for(int i = 0; i < gpio__nTerminals; ++i) {
				snprintf(
					msgOutBuf, sizeof(msgOutBuf), "%d", gpio__getTerminalNo(i)
					);
				usbcdc__sendStringNoFlush(msgOutBuf);
				if(i < gpio__nTerminals - 1)
					usbcdc__sendStringNoFlush(",");
				}
			usbcdc__sendString("\r\n");
			}
		else if(!strcmp(command.mnem, "TCP")) {
			int terminalNo = command.leftArgs[0].uint8Val;
			int supportsOutput = gpio__supportsOutput(terminalNo);
			int supportsInput = gpio__supportsInput(terminalNo);
			if (supportsOutput < 0 || supportsInput < 0) {
				usbcdc__sendString("ERROR:VAL\r\n");
				}
			else {
				snprintf(
					msgOutBuf,
					sizeof(msgOutBuf),
					"TCP:%d=",
					command.leftArgs[0].uint8Val
					);
				usbcdc__sendStringNoFlush(msgOutBuf);
				if(supportsInput)
					usbcdc__sendStringNoFlush("I");
				if(supportsOutput)
					usbcdc__sendStringNoFlush("O");
				usbcdc__sendString("\r\n");
				}
			}
		else if(!strcmp(command.mnem, "IDN")) {
			usbcdc__sendString("IDN=FOO,BAR\r\n"); // TODO
			}
		else if(command.cmdType == CMDTYPE_QUERY) {
			if(!strcmp(command.mnem, "OUT")) {
				cmdResult = gpio__getOutput(command.leftArgs[0].uint8Val);
				}
			else if(!strcmp(command.mnem, "INP")) {
				cmdResult = gpio__getInput(command.leftArgs[0].uint8Val);
				}
			else if(!strcmp(command.mnem, "DIR")) {
				cmdResult = gpio__getDirection(command.leftArgs[0].uint8Val);
				}
			else {
				usbcdc__sendString("ERROR:CMD\r\n");
				abort = 1;
				}
			if(!abort) {
				if(cmdResult < 0) {
					usbcdc__sendString("ERROR:VAL\r\n");
					}
				else {
					snprintf(
						msgOutBuf,
						sizeof(msgOutBuf),
						"%s:%d=%d\r\n",
						command.mnem,
						command.leftArgs[0].uint8Val,
						cmdResult
						);
					usbcdc__sendString(msgOutBuf);
					}
				}
			}
		else if(command.cmdType == CMDTYPE_SET) {
			if(!strcmp(command.mnem, "OUT")) {
				cmdResult = gpio__setOutput(
					command.leftArgs[0].uint8Val,
					command.rightArgs[0].uint8Val
					);
				}
			else if(!strcmp(command.mnem, "DIR")) {
				cmdResult = gpio__setDirection(
					command.leftArgs[0].uint8Val,
					command.rightArgs[0].uint8Val
					);
				}
			else {
				usbcdc__sendString("ERROR:CMD\r\n");
				abort = 1;
				}
			if(!abort) {
				if(cmdResult) {
					usbcdc__sendString("ERROR:VAL\r\n");
					}
				else {
					usbcdc__sendString("OK\r\n");
					}
				}
			}
		else {
			usbcdc__sendString("ERROR:CMD\r\n");
			}
		}
	while(usbcdc__hasInputWaiting()) {
		int16_t ch = usbcdc__getNextInputChar();
		if(ch >= 0)
			cmdproc__processIncomingChar((uint8_t)ch);
		}
	}

void mstick__tick(volatile uint16_t *tickCounter) {
	statusleds__onMsTick(tickCounter);
	}

int main(void) {
	hwInit();
	statusleds__init();
	gpio__init();
	cmdproc__init();
	mstick__init();
	usbcdc__init();
	sei();

	while(1) {
		usbcdc__task();
		statusleds__task();
		handleCommand();
		}
	}
