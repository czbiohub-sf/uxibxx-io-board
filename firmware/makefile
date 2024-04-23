CC = avr-gcc
OBJCOPY = avr-objcopy

LUFA_OPTS = -ILUFA-210130 -DF_USB=16000000 -DUSB_DEVICE_ONLY -DARCH=ARCH_AVR8
CCOPTS = -DF_CPU=16000000 -O1 -std=gnu99 -Wstrict-prototypes -fshort-enums \
	-fno-inline-small-functions -Wall -fno-strict-aliasing -funsigned-char \
	-funsigned-bitfields -ffunction-sections $(LUFA_OPTS)
LUFA_DIR = LUFA-210130/LUFA
MCU = atmega32u4
DFUP_TARGET = atmega32u4

CC_CMD = $(CC) -mmcu=$(MCU) $(CCOPTS)

TARGET = main
OBJS = main.o mstick.o statusleds.o usbcdc.o usbcdc_descriptors.o cmdproc.o \
	commands.o gpio.o
LUFA_CORE_OBJS = USBTask.o Events.o DeviceStandardReq.o 
LUFA_AVR_OBJS = Device_AVR8.o USBController_AVR8.o USBInterrupt_AVR8.o \
	Endpoint_AVR8.o EndpointStream_AVR8.o
LUFA_CLS_DRVR_OBJS = CDCClassDevice.o 
LUFA_OBJS = $(LUFA_CORE_OBJS) $(LUFA_AVR_OBJS) $(LUFA_CLS_DRVR_OBJS)

.PHONY: all hex elf
all: hex
hex: $(TARGET).hex
elf: $(TARGET).elf


$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $(TARGET).elf $(TARGET).hex

$(TARGET).elf: $(LUFA_OBJS) $(OBJS)
	$(CC) -mmcu=$(MCU) -o $(TARGET).elf $(LUFA_OBJS) $(OBJS)

$(OBJS): %.o: src/%.c
	$(CC_CMD) -o $@ -c $<

$(LUFA_CORE_OBJS): %.o: $(LUFA_DIR)/Drivers/USB/Core/%.c
	$(CC_CMD) -o $@ -c $<

$(LUFA_AVR_OBJS): %.o: $(LUFA_DIR)/Drivers/USB/Core/AVR8/%.c
	$(CC_CMD) -o $@ -c $<

$(LUFA_CLS_DRVR_OBJS): %.o: $(LUFA_DIR)/Drivers/USB/Class/Device/%.c
	$(CC_CMD) -o $@ -c $<

flash: $(TARGET).hex
	sudo dfu-programmer $(DFUP_TARGET) erase
	sudo dfu-programmer $(DFUP_TARGET) flash $(TARGET).hex
	sudo dfu-programmer $(DFUP_TARGET) start

clean:
	rm -f $(OBJS) $(LUFA_OBJS) $(TARGET).hex $(TARGET).elf
