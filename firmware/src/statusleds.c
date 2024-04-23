#include <stdint.h>
#include <avr/io.h>


#define LED_HBT_PORT PORTD
#define LED_HBT_DDR DDRD
#define LED_HBT_BM _BV(PD5)
#define LED_USB_PORT PORTB
#define LED_USB_DDR DDRB
#define LED_USB_BM _BV(PB0)

#define HEARTBEAT_HALFPERIOD_MS 500
#define WINK_DURATION_MS 60
#define WINK_RECOVERY_MS 40


volatile uint16_t currentTick;
volatile uint16_t lastHbtChangeTick;
volatile uint16_t winkStartTick;
volatile struct {
	unsigned int winking :1;
	unsigned int winkCooldown :1;
	unsigned int hbt :1;
	} volFlags;
struct {
	unsigned int usbLedSetting :1;
	unsigned int hbtAck :1;
	unsigned int startWink :1;
	} flags;


inline void setHbtLed(int on) {
	if(on) {
		LED_HBT_PORT &= ~LED_HBT_BM;
		}
	else {
		LED_HBT_PORT |= LED_HBT_BM;
		}
	}

inline void toggleHbtLed(void) {
	LED_HBT_PORT ^= LED_HBT_BM;
	}

inline void setUsbLed(int on) {
	if(on) {
		LED_USB_PORT &= ~LED_USB_BM;
		}
	else {
		LED_USB_PORT |= LED_USB_BM;
		}
	}

void statusleds__init(void) {
	setHbtLed(0);
	setUsbLed(0);
	LED_HBT_DDR |= LED_HBT_BM;
	LED_USB_DDR |= LED_USB_BM;
	lastHbtChangeTick = 0;
	volFlags.winking = 0;
	volFlags.winkCooldown = 0;
	volFlags.hbt = 0;
	flags.usbLedSetting = 0;
	flags.hbtAck = 0;
	flags.startWink = 0;
	}

void statusleds__setHbtLed(int on) {
	setHbtLed(on);
	}

void statusleds__setUsbLed(int on) {
	flags.usbLedSetting = !!on;
	setUsbLed(on);
	}

void statusleds__winkUsbLed(void) {
	flags.startWink = 1;
	}

void statusleds__onMsTick(volatile uint16_t *msCounter) {
	currentTick = *msCounter;
	if(flags.hbtAck) {
		volFlags.hbt = 0;
		}
	else if(currentTick - lastHbtChangeTick >= HEARTBEAT_HALFPERIOD_MS) {
		volFlags.hbt = 1;
		lastHbtChangeTick = currentTick;
		}
	if(volFlags.winkCooldown) {
		if(currentTick - winkStartTick >= WINK_DURATION_MS + WINK_RECOVERY_MS) {
			volFlags.winking = 0;
			volFlags.winkCooldown = 0;
			}
		}
	else if(volFlags.winking) {
		if(currentTick - winkStartTick >= WINK_DURATION_MS) {
			setUsbLed(flags.usbLedSetting);
			volFlags.winkCooldown = 1;
			}
		}
	else if(flags.startWink) {
		winkStartTick = currentTick;
		setUsbLed(0);
		volFlags.winking = 1;
		}
	}

void statusleds__task(void) {
	if(flags.hbtAck) {
		if(!volFlags.hbt)
			flags.hbtAck = 0;
		}
	else if(volFlags.hbt) {
		toggleHbtLed();
		flags.hbtAck = 1;
		}
	if(flags.startWink) {
		if(volFlags.winking)
			flags.startWink = 0;
		}
	}