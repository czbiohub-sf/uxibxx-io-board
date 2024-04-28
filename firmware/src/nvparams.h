#define BOARDID_LEN_MAX 16


typedef struct {
	uint8_t boardId[BOARDID_LEN_MAX + 1];
	uint16_t crc;
	} nvparams_t;


void nvparams__loadDefaults(nvparams_t *dest);
int nvparams__load(nvparams_t *dest);
void nvparams__init(nvparams_t *dest);
void nvparams__save(nvparams_t *src);
