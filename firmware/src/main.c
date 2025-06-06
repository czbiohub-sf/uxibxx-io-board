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
#include "nvparams.h"
#include "statusleds.h"
#include "usbcdc.h"
#include "board_info.h"


#define BOOTLOADER_TRIGGER_KEY 0xCB49


static uint16_t blJumpTrigger __attribute__((section (".noinit")));
static nvparams_t nvParams;


// Misc subroutines

void maybeJumpToBootloader(void) {
	if((MCUSR & _BV(WDRF)) && (blJumpTrigger == BOOTLOADER_TRIGGER_KEY)) {
		asm volatile("jmp 0x7000"::);
		}
	blJumpTrigger = 0;
	}

void triggerWatchdogReset(void) {
	wdt_enable(WDTO_2S);
	_delay_ms(1000);
	// ^ courtesy delay for host application to cleanly close file handle
	cli();
	usbcdc__detach();
	statusleds__setHbtLed(1);
	statusleds__setUsbLed(0);
	while(1) {}
	}

void triggerResetToApp(void) {
	triggerWatchdogReset();
	}

void triggerResetToBootloader(void) {
	blJumpTrigger = BOOTLOADER_TRIGGER_KEY;
	triggerWatchdogReset();
	}

void hwInit(void) {
	maybeJumpToBootloader();
	MCUSR = 0;
	wdt_disable();
	clock_prescale_set(clock_div_1);
	}

void handleCommand(void) {
	cmdproc_command_t command;
	char msgOutBuf[33];
	int cmdResult;
	int abort = 0;

	if(cmdproc__hasCommandWaiting()) {
		statusleds__winkUsbLed();
		cmdproc__getCommand(&command);
		if(command.parseError) {
			switch(command.parseError){
				case ERROR_CMD:
					usbcdc__sendString("ERROR:CMD\r\n");
					break;
				case ERROR_N_ARGS:
					usbcdc__sendString("ERROR:ARGN\r\n");
					break;
				case ERROR_ARG_FMT:
					usbcdc__sendString("ERROR:ARGFMT\r\n");
					break;
				case ERROR_ARG_VAL:
					usbcdc__sendString("ERROR:ARGVAL\r\n");
					break;
				default:
					usbcdc__sendString("ERROR:UNK\r\n");
				}
			}
		else if(!strcmp(command.mnem, "DFU")) {
			usbcdc__sendString("OK\r\n");
			triggerResetToBootloader();
			}
		else if(!strcmp(command.mnem, "RST")) {
			usbcdc__sendString("OK\r\n");
			triggerResetToApp();
			}
		else if(!strcmp(command.mnem, "TLS")) {
			usbcdc__sendStringNoFlush("TLS=");
			for(int i = 0; i < gpio__nTerminals; ++i) {
				snprintf(
					msgOutBuf,
					sizeof(msgOutBuf) - 1,
					"%d",
					gpio__getTerminalNo(i)
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
			snprintf(
				msgOutBuf,
				sizeof(msgOutBuf),
				"IDN=%s,%s\r\n",
				BOARD_MODEL_STR,
				nvParams.boardId
				);
			usbcdc__sendString(msgOutBuf);
			}
		else if(!strcmp(command.mnem, "SER")) {
			strncpy(
				nvParams.boardId,
				command.rightArgs[0].stringVal,
				BOARDID_LEN_MAX
				);
			usbcdc__sendString("OK\r\n");
			}
		else if(!strcmp(command.mnem, "NVS")) {
			nvparams__save(&nvParams);
			usbcdc__sendString("OK\r\n");
			}
		else if(!strcmp(command.mnem, "NVL")) {
			nvparams__load(&nvParams);
			usbcdc__sendString("OK\r\n");
			}
		else if(!strcmp(command.mnem, "DEF")) {
			nvparams__loadDefaults(&nvParams);
			usbcdc__sendString("OK\r\n");
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
				usbcdc__sendString("ERROR:IMP\r\n");
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
				usbcdc__sendString("ERROR:IMP\r\n");
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
			usbcdc__sendString("ERROR:IMP\r\n");
			}
		}
	while(usbcdc__hasInputWaiting()) {
		int16_t ch = usbcdc__getNextInputChar();
		if(ch >= 0)
			cmdproc__processIncomingChar((uint8_t)ch);
		}
	}

void mstick__tickEvent(volatile uint16_t *tickCounter) {
	statusleds__onMsTick(tickCounter);
	}

	}


// Main routine (unnecessary section title)
	
int main(void) {
	hwInit();
	statusleds__init();
	gpio__init();
	nvparams__init(&nvParams);
	cmdproc__init();
	mstick__init();
	usbcdc__init(nvParams.boardId);
	sei();

	while(1) {
		usbcdc__task();
		statusleds__task();
		handleCommand();
		}
	}
