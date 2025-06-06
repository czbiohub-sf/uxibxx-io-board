#include <stddef.h>
#include <avr/io.h>
#include <util/atomic.h>

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

static const gpio_terminal_def_t gpioTerminalDefs[] = {
	{1,  &DDRB, NULL,  &PORTB, PB6, DIR_OUT, 0},
	{2,  &DDRB, NULL,  &PORTB, PB2, DIR_OUT, 0},
	{3,  &DDRB, NULL,  &PORTB, PB3, DIR_OUT, 0},
	{4,  &DDRB, NULL,  &PORTB, PB1, DIR_OUT, 0},
	{5,  &DDRF, NULL,  &PORTF, PF7, DIR_OUT, 0},
	{6,  &DDRF, NULL,  &PORTF, PF6, DIR_OUT, 0},
	{7,  &DDRB, NULL,  &PORTB, PB4, DIR_OUT, 0},
	{8,  &DDRE, NULL,  &PORTE, PE6, DIR_OUT, 0},
	{9,  &DDRC, NULL,  &PORTC, PC6, DIR_OUT, 0},
	{10, &DDRD, NULL,  &PORTD, PD4, DIR_OUT, 0},
	{11, &DDRF, NULL,  &PORTF, PF4, DIR_OUT, 0},
	{12, &DDRF, NULL,  &PORTF, PF5, DIR_OUT, 0},
	{13, &DDRD, &PIND, &PORTD, PD7, DIR_IN,  0},
	{14, &DDRB, &PINB, &PORTB, PB5, DIR_IN,  0},
	};

const int gpio__nTerminals =
	sizeof(gpioTerminalDefs) / sizeof(gpio_terminal_def_t);


const gpio_terminal_def_t *getTerminal(int terminalNo) {
	for(int i = 0; i < gpio__nTerminals; ++i) {
		if(gpioTerminalDefs[i].terminalNo == terminalNo)
			return &gpioTerminalDefs[i];
		}
	return NULL;
	}

inline int readIoRegBitIndirect(const volatile uint8_t *reg, int bitNo) {
	return !!(*reg & _BV(bitNo));
	}

inline void setIoRegBitIndirect(volatile uint8_t *reg, int bitNo, int on) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// SBI instruction operands must be immediate so this is always going
		// to be a multi-instruction operation unless we get really silly
		if(on) {
			*reg |= _BV(bitNo);
			}
		else {
			*reg &= ~_BV(bitNo);
			}
		}
	}

inline enum gpio_terminal_dir getDirection(
		const gpio_terminal_def_t *terminal) {
	return readIoRegBitIndirect(terminal->dirReg, terminal->ioBit);
	}

inline int getOutput(const gpio_terminal_def_t *terminal) {
	return readIoRegBitIndirect(terminal->outputReg, terminal->ioBit);
	}

inline int getInput(const gpio_terminal_def_t *terminal) {
	return readIoRegBitIndirect(terminal->inputReg, terminal->ioBit);
	}

void gpio__init(void) {
	for(int i = 0; i < gpio__nTerminals; ++i) {
		const gpio_terminal_def_t *term = &gpioTerminalDefs[i];
		if(term->dirReg)
			setIoRegBitIndirect(
				term->dirReg, term->ioBit, term->initialDir == DIR_OUT);
		if(term->outputReg)
			setIoRegBitIndirect(term->outputReg, term->ioBit, term->initialOutput);
		}
	}

int gpio__getInput(int terminalNo) {
	const gpio_terminal_def_t *terminal = getTerminal(terminalNo);
	if(!terminal)
		return -1;
	if(!terminal->inputReg)
		return -1;
	return getInput(terminal);
	}

int gpio__supportsOutput(int terminalNo) {
	const gpio_terminal_def_t *terminal = getTerminal(terminalNo);
	if(!terminal)
		return -1;
	return !!terminal->outputReg;
	}

int gpio__supportsInput(int terminalNo) {
	const gpio_terminal_def_t *terminal = getTerminal(terminalNo);
	if(!terminal)
		return -1;
	return !!terminal->inputReg;
	}

int gpio__getTerminalNo(int terminalIdx) {
	if(terminalIdx >= gpio__nTerminals)
		return -1;
	return gpioTerminalDefs[terminalIdx].terminalNo;
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
	setIoRegBitIndirect(terminal->outputReg, terminal->ioBit, on);
	return 0;
	}

int gpio__toggleOutput(int terminalNo) {
	int prev = gpio__getOutput(terminalNo);
	if(prev < 0)
		return prev;
	return gpio__setOutput(terminalNo, !prev);
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
	setIoRegBitIndirect(terminal->dirReg, terminal->ioBit, dir);
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
