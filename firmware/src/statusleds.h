#pragma once


#include <stdint.h>


void statusleds__init(void);
void statusleds__setHbtLed(int on);
void statusleds__setUsbLed(int on);
void statusleds__winkUsbLed(void);
void statusleds__onMsTick(volatile uint16_t *msCounter);
void statusleds__task(void);
