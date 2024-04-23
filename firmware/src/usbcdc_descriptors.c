#include <avr/pgmspace.h>
#include <LUFA/Drivers/USB/USB.h>

#include "usbcdc_config.h"


#define STRINGID_LANG 0
#define STRINGID_MFR 1
#define STRINGID_PROD 2
#define STRINGID_SERIAL 3


const USB_Descriptor_String_t PROGMEM langString = 
	USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
const USB_Descriptor_String_t PROGMEM mfrString =
	USB_STRING_DESCRIPTOR(MFR_STRING);
const USB_Descriptor_String_t PROGMEM prodString =
	USB_STRING_DESCRIPTOR(PROD_STRING);
USB_Descriptor_String_t serialNo =
	USB_STRING_DESCRIPTOR(L"One two three"); //TODO

const USB_Descriptor_Device_t PROGMEM deviceDescriptor = {
	.Header = {
		.Size = sizeof(USB_Descriptor_Device_t),
		.Type = DTYPE_Device,
		},
	.USBSpecification = VERSION_BCD(1, 1, 0),
	.Class = CDC_CSCP_CDCClass,
	.SubClass = CDC_CSCP_NoSpecificSubclass,
	.Protocol = CDC_CSCP_NoSpecificProtocol,
	.Endpoint0Size = ENDPOINT_CONTROLEP_DEFAULT_SIZE,
	.VendorID = VENDOR_ID,
	.ProductID = PRODUCT_ID,
	.ReleaseNumber = RELEASENUMBER,
	.ManufacturerStrIndex = STRINGID_MFR,
	.ProductStrIndex = STRINGID_PROD,
	.SerialNumStrIndex = STRINGID_SERIAL,
	.NumberOfConfigurations = 1
	};

struct config_descriptor {
	USB_Descriptor_Configuration_Header_t configHeader;
	USB_Descriptor_Interface_t cciInterface;
	USB_CDC_Descriptor_FunctionalHeader_t cciHeaderFunctional;
	USB_CDC_Descriptor_FunctionalACM_t acmFunctional;
	USB_CDC_Descriptor_FunctionalUnion_t unionFunctional;
	USB_Descriptor_Endpoint_t notificationEp;
	USB_Descriptor_Interface_t dciInterface;
	USB_Descriptor_Endpoint_t dataOutEp;
	USB_Descriptor_Endpoint_t dataInEp;
	};

const struct config_descriptor PROGMEM configDescriptor = {
	.configHeader = {
		.Header = {
			.Size = sizeof(USB_Descriptor_Configuration_Header_t),
			.Type = DTYPE_Configuration,
			},
		.ConfigAttributes = USB_CONFIG_ATTR_RESERVED,
		.ConfigurationNumber = 1,
		.ConfigurationStrIndex = NO_DESCRIPTOR,
		.MaxPowerConsumption = USB_CONFIG_POWER_MA(50),
		.TotalConfigurationSize = sizeof(struct config_descriptor),
		.TotalInterfaces = 2,
		},
	.cciInterface = {
		.Header = {
			.Size = sizeof(USB_Descriptor_Interface_t),
			.Type = DTYPE_Interface,
			},
		.AlternateSetting = 0,
		.InterfaceNumber = 0,
		.InterfaceStrIndex = NO_DESCRIPTOR,
		.Class = CDC_CSCP_CDCClass,
		.SubClass = CDC_CSCP_ACMSubclass,
		.Protocol = CDC_CSCP_NoSpecificProtocol,
		.TotalEndpoints = 1,
		},
	.cciHeaderFunctional = {
		.Header = {
			.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t),
			.Type = CDC_DTYPE_CSInterface,
			},
		.Subtype = CDC_DSUBTYPE_CSInterface_Header,
		.CDCSpecification = VERSION_BCD(1,1,0),
		},
	.acmFunctional = {
		.Header = {
			.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t),
			.Type = CDC_DTYPE_CSInterface,
			},
		.Subtype = CDC_DSUBTYPE_CSInterface_ACM,
		.Capabilities = 0x06,
		},
	.unionFunctional = {
		.Header = {
			.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t),
			.Type = CDC_DTYPE_CSInterface,
			},
		.Subtype = CDC_DSUBTYPE_CSInterface_Union,
		.MasterInterfaceNumber = 0,
		.SlaveInterfaceNumber = 1,
		},
	.notificationEp = {
		.Header = {
			.Size = sizeof(USB_Descriptor_Endpoint_t),
			.Type = DTYPE_Endpoint,
			},
		.Attributes = 
			EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA,
		.EndpointAddress = NOTIF_EP_ADDR,
		.EndpointSize = NOTIF_EP_SIZE,
		.PollingIntervalMS = 0xFF,
		},
	.dciInterface = {
		.Header = {
			.Size = sizeof(USB_Descriptor_Interface_t),
			.Type = DTYPE_Interface,
			},
		.AlternateSetting = 0,
		.InterfaceNumber = 1,
		.InterfaceStrIndex = NO_DESCRIPTOR,
		.Class = CDC_CSCP_CDCDataClass,
		.SubClass = CDC_CSCP_NoDataSubclass,
		.Protocol = CDC_CSCP_NoDataProtocol,
		.TotalEndpoints = 2,
		},
	.dataOutEp = {
		.Header = {
			.Size = sizeof(USB_Descriptor_Endpoint_t),
			.Type = DTYPE_Endpoint,
			},
		.EndpointAddress = DOUT_EP_ADDR,
		.EndpointSize = DOUT_EP_SIZE,
		.Attributes = 
			EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA,
		},
	.dataInEp = {
		.Header = {
			.Size = sizeof(USB_Descriptor_Endpoint_t),
			.Type = DTYPE_Endpoint,
			},
		.EndpointAddress = DIN_EP_ADDR,
		.EndpointSize = DIN_EP_SIZE,
		.Attributes = 
			EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA,
		},
	};


uint16_t CALLBACK_USB_GetDescriptor(
			const uint16_t wValue,
			const uint16_t wIndex,
			const void** const descriptorAddress,
			uint8_t* const descriptorMemorySpace) {
	uint8_t descriptorIdx = wValue & 0xFF;
	uint8_t descriptorType = wValue >> 8;

	switch(descriptorType) {
		case DTYPE_Device:
			*descriptorAddress = &deviceDescriptor;
			*descriptorMemorySpace = MEMSPACE_FLASH;
			return sizeof(USB_Descriptor_Device_t);
		case DTYPE_String:
			switch(descriptorIdx) {
				case STRINGID_LANG:
					*descriptorAddress = &langString;
					*descriptorMemorySpace = MEMSPACE_FLASH;
					return pgm_read_byte(&langString.Header.Size);
				case STRINGID_MFR:
					*descriptorAddress = &mfrString;
					*descriptorMemorySpace = MEMSPACE_FLASH;
					return pgm_read_byte(&mfrString.Header.Size);
				case STRINGID_PROD:
					*descriptorAddress = &prodString;
					*descriptorMemorySpace = MEMSPACE_FLASH;
					return pgm_read_byte(&prodString.Header.Size);
				case STRINGID_SERIAL:
					*descriptorAddress = &serialNo;
					*descriptorMemorySpace = MEMSPACE_RAM;
					return serialNo.Header.Size;
				}
		case DTYPE_Configuration:
			*descriptorAddress = &configDescriptor;
			*descriptorMemorySpace = MEMSPACE_FLASH;
			return sizeof(struct config_descriptor);
		}
	*descriptorAddress = NULL;
	return NO_DESCRIPTOR;
	}
