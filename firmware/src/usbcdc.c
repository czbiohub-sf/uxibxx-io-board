#include <LUFA/Drivers/USB/USB.h>

#include "statusleds.h"
#include "usbcdc_config.h"
#include "usbcdc.h"


USB_ClassInfo_CDC_Device_t cdcInterface = {
	.Config = {
		.ControlInterfaceNumber = 0,
		.NotificationEndpoint = {
			.Address = NOTIF_EP_ADDR,
			.Size = NOTIF_EP_SIZE,
			.Banks = 1,
			},
		.DataINEndpoint = {
			.Address = DIN_EP_ADDR,
			.Size = DIN_EP_SIZE,
			.Banks = 1,
			},
		.DataOUTEndpoint = {
			.Address = DOUT_EP_ADDR,
			.Size = DOUT_EP_SIZE,
			.Banks = 1,
			},
		},
	};



void usbcdc__init(void) {
	USB_Init(
		USB_OPT_REG_ENABLED | USB_OPT_AUTO_PLL | USB_DEVICE_OPT_FULLSPEED);
	}

void usbcdc__task(void) {
	USB_USBTask();
	CDC_Device_USBTask(&cdcInterface);
	}

void usbcdc__detach(void) {
	USB_Detach();
	}

int usbcdc__hasInputWaiting(void) {
	return !!CDC_Device_BytesReceived(&cdcInterface);
	}

int16_t usbcdc__getNextInputChar(void) {
	return CDC_Device_ReceiveByte(&cdcInterface);
	}

void usbcdc__sendString(const char *str) {
	usbcdc__sendStringNoFlush(str);
	CDC_Device_Flush(&cdcInterface);
	}

void usbcdc__sendStringNoFlush(const char *str) {
	CDC_Device_SendString(&cdcInterface, str);
	}

void EVENT_USB_Device_ControlRequest(void) {
	CDC_Device_ProcessControlRequest(&cdcInterface);
	}

void EVENT_USB_Device_Connect(void) {
	statusleds__setUsbLed(0);
	}

void EVENT_USB_Device_Disconnect(void) {
	statusleds__setUsbLed(0);
	}

void EVENT_USB_Device_ConfigurationChanged(void) {
	statusleds__setUsbLed(0);
	int result = CDC_Device_ConfigureEndpoints(&cdcInterface);
	if(result)
		statusleds__setUsbLed(1);
	}
