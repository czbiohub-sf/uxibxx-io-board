#pragma once


#define MFR_STRING L"CZBSF BioE"
#define PROD_STRING L"UXIB-DN12 12-channel solenoid controller"
#define VENDOR_ID 0x4743
#define PRODUCT_ID 0xB499
#define RELEASENUMBER VERSION_BCD(0,0,1)

#define CONTROL_EP_SIZE 8

#define NOTIF_EP_ADDR (ENDPOINT_DIR_IN | 1)
#define NOTIF_EP_SIZE 8

#define DIN_EP_ADDR (ENDPOINT_DIR_IN | 2)
#define DOUT_EP_ADDR (ENDPOINT_DIR_OUT | 3)
#define DATA_IO_EP_SIZE 16
#define DIN_EP_SIZE DATA_IO_EP_SIZE
#define DOUT_EP_SIZE DATA_IO_EP_SIZE
