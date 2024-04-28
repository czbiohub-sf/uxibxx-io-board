#include <stdint.h>
#include <string.h>

#include <avr/eeprom.h>
#include <util/crc16.h>

#include "board_info.h"
#include "nvparams.h"


#define EEPROM_START_OFFS 0


uint16_t calculateCrc(void *start, int nBytes) {
	uint16_t crc = 0xFFFF;
	for(int i = 0; i < nBytes; ++i)
		crc = _crc16_update(crc, *((uint8_t *)start + i));
	return crc;
	}

void nvparams__loadDefaults(nvparams_t *dest) {
	strncpy((char *)dest->boardId, DEFAULT_SERIALNO_STR, BOARDID_LEN_MAX);
	dest->boardId[BOARDID_LEN_MAX - 1] = 0;
	}

int nvparams__load(nvparams_t *dest) {
	nvparams_t buf;
	eeprom_read_block(&buf, EEPROM_START_OFFS, sizeof(nvparams_t));
	if(calculateCrc(&buf, sizeof(nvparams_t) - sizeof(uint16_t)) != buf.crc)
		return -1;
	*dest = buf;
	return 0;
	}

void nvparams__init(nvparams_t *dest) {
	if(nvparams__load(dest) < 0)
		nvparams__loadDefaults(dest);
	}

void nvparams__save(nvparams_t *src) {
	src->crc =
		calculateCrc(src, sizeof(nvparams_t) - sizeof(uint16_t));
	eeprom_write_block(src, EEPROM_START_OFFS, sizeof(nvparams_t));
	}
