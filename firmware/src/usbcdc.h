#pragma once


#include <stdint.h>


void usbcdc__init(const char *serNo);
void usbcdc__task(void);
void usbcdc__detach(void);
void usbcdc__initSerialNo(const char *serNo);
int usbcdc__hasInputWaiting(void);
int16_t usbcdc__getNextInputChar(void);
void usbcdc__sendString(const char *str);
void usbcdc__sendStringNoFlush(const char *str);
