#include <stdint.h>

#include <avr/pgmspace.h>


#define CMDPROC_MNEM_MAX_LEN 16
#define CMDPROC_ARG_MAX_LEN 16
#define CMDPROC_MAX_N_LEFTARGS 1
#define CMDPROC_MAX_N_RIGHTARGS 1

typedef enum {
	ERROR_CMD = 1,
	ERROR_N_ARGS = 2,
	ERROR_ARG_FMT = 4,
	ERROR_ARG_VAL = 8,
	} cmdproc_error_t;

typedef enum {
	CMDTYPE_DO = 1,
	CMDTYPE_QUERY = 2,
	CMDTYPE_SET = 3
	} cmdproc_cmdtype_t;

typedef enum {
	ARGTYPE_STRING,
	ARGTYPE_UINT8,
	ARGTYPE_UINT16,
	ARGTYPE_INT8,
	ARGTYPE_INT16,
	} cmdproc_argtype_t;

typedef union {
	uint8_t uint8Val;
	uint16_t uint16Val;
	int8_t int8Val;
	int16_t int16Val;
	uint8_t stringVal[CMDPROC_ARG_MAX_LEN + 1];
	} cmdproc_argval_t;

typedef struct {
	cmdproc_cmdtype_t cmdType;
	char mnem[CMDPROC_MNEM_MAX_LEN + 1];
	int nLeftArgs;
	int nRightArgs;
	cmdproc_argval_t leftArgs[CMDPROC_MAX_N_LEFTARGS];
	cmdproc_argval_t rightArgs[CMDPROC_MAX_N_RIGHTARGS];
	cmdproc_error_t parseError;
	} cmdproc_command_t;

typedef struct {
	cmdproc_cmdtype_t cmdType;
	char mnem[CMDPROC_MNEM_MAX_LEN + 1];
	int nLeftArgs;
	int nRightArgs;
	cmdproc_argtype_t leftArgTypes[CMDPROC_MAX_N_LEFTARGS];
	cmdproc_argtype_t rightArgTypes[CMDPROC_MAX_N_RIGHTARGS];
	} cmdproc_cmd_spec_t;


extern const cmdproc_cmd_spec_t PROGMEM cmdproc__commandSpecs[];
extern const int cmdproc__commandSpecsLen;


void cmdproc__init(void);
void cmdproc__processIncomingChar(uint8_t ch);
int cmdproc__hasCommandWaiting(void);
cmdproc_error_t cmdproc__getCommand(cmdproc_command_t *dest);
