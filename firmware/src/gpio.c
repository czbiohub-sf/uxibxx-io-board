#include <stddef.h>
#include <avr/io.h>

#include "gpio.h"


typedef struct {
	uint8_t terminalNo;
	volatile uint8_t *dirReg;
	volatile uint8_t *inputReg;
	volatile uint8_t *outputReg;
	int ioBit;
	int initialDir;
	int initialOutput;
	} gpio_terminal_def_t;

// TODO fill in actual values
static const gpio_terminal_def_t gpioTerminalDefs[] = {
	{1, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{2, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{3, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{4, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{5, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{6, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{7, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{8, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{9, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{10, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{11, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{12, &DDRD, NULL, &PORTD, PD5, DIR_OUT, 0},
	{13, &DDRB, &PINB, &PORTB, PB5, DIR_IN, 0},
	{14, &DDRB, &PINB, &PORTB, PB6, DIR_IN, 0},
	};
static const int nTerminals =
	sizeof(gpioTerminalDefs) / sizeof(gpio_terminal_def_t);


const gpio_terminal_def_t *getTerminal(int terminalNo) {
	for(int i = 0; i < nTerminals; ++i) {
		if(gpioTerminalDefs[i].terminalNo == terminalNo)
			return &gpioTerminalDefs[i];
		}
	return NULL;
	}

inline void setIoRegBit(volatile uint8_t *reg, int bitNo, int on) {
	if(on) {
		*reg |= _BV(bitNo);
		}
	else {
		*reg &= ~_BV(bitNo);
		}
	}

void gpio__init(void) {
	for(int i = 0; i < nTerminals; ++i) {
		const gpio_terminal_def_t *term = &gpioTerminalDefs[i];
		if(term->dirReg)
			setIoRegBit(
				term->dirReg, term->ioBit, term->initialDir == DIR_OUT);
		if(term->outputReg)
			setIoRegBit(term->outputReg, term->ioBit, term->initialOutput);
		}
	}

inline enum gpio_terminal_dir getDirection(
		const gpio_terminal_def_t *terminal) {
	return !!(*terminal->dirReg & _BV(terminal->ioBit));
	}

inline int getOutput(const gpio_terminal_def_t *terminal) {
	return !!(*terminal->outputReg & _BV(terminal->ioBit));
	}

inline int getInput(const gpio_terminal_def_t *terminal) {
	return !!(*terminal->inputReg & _BV(terminal->ioBit));
	}

int gpio__getInput(int terminalNo) {
	const gpio_terminal_def_t *terminal = getTerminal(terminalNo);
	if(!terminal)
		return -1;
	if(!terminal->inputReg)
		return -1;
	return getInput(terminal);
	}

int gpio__getOutput(int terminalNo) {
	const gpio_terminal_def_t *terminal = getTerminal(terminalNo);
	if(!terminal)
		return -1;
	if(!terminal->outputReg)
		return -1;
	return getOutput(terminal);
	}

int gpio__setOutput(int terminalNo, int on) {
	//TODO: have different error codes for invalid terminal number, unsupported operation/mode, etc.
	const gpio_terminal_def_t *terminal = getTerminal(terminalNo);
	if(!terminal)
		return -1;
	if(!terminal->outputReg)
		return -1;
	setIoRegBit(terminal->outputReg, terminal->ioBit, on);
	return 0;
	}

int gpio__setDirection(int terminalNo, enum gpio_terminal_dir dir) {
	const gpio_terminal_def_t *terminal = getTerminal(terminalNo);
	if(!terminal)
		return -1;
	if(!terminal->dirReg)
		return -1;
	if((dir == DIR_OUT) && !terminal->outputReg)
		return -1;
	if((dir == DIR_IN) && !terminal->inputReg)
		return -1;
	setIoRegBit(terminal->dirReg, terminal->ioBit, dir);
	return 0;
	}

int gpio__getDirection(int terminalNo) {
	const gpio_terminal_def_t *terminal = getTerminal(terminalNo);
	if(!terminal)
		return -1;
	if(!terminal->dirReg)
		return -1;
	return getDirection(terminal);
	}
