#include <stdint.h>


#define CMDPROC_MNEM_MAX_LEN 16
#define CMDPROC_ARG_MAX_LEN 16
#define CMDPROC_MAX_N_LEFTARGS 1
#define CMDPROC_MAX_N_RIGHTARGS 1


typedef enum {
	CMDTYPE_DO = 1,
	CMDTYPE_QUERY = 2,
	CMDTYPE_SET = 3
	} cmdproc_cmdtype_t;

typedef enum {
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
	} cmdproc_argval_t;

typedef struct {
	cmdproc_cmdtype_t cmdType;
	char mnem[CMDPROC_MNEM_MAX_LEN + 1];
	int nLeftArgs;
	int nRightArgs;
	cmdproc_argval_t leftArgs[CMDPROC_MAX_N_LEFTARGS];
	cmdproc_argval_t rightArgs[CMDPROC_MAX_N_LEFTARGS];
	int parseError;
	} cmdproc_command_t;

typedef struct {
	int cmdId;
	cmdproc_cmdtype_t cmdType;
	char mnem[CMDPROC_MNEM_MAX_LEN + 1];
	int nLeftArgs;
	int nRightArgs;
	cmdproc_argtype_t leftArgTypes[CMDPROC_MAX_N_LEFTARGS];
	cmdproc_argtype_t rightArgTypes[CMDPROC_MAX_N_LEFTARGS];
	} cmdproc_cmd_spec_t;


void cmdproc__init(void);
void cmdproc__processIncomingChar(uint8_t ch);
int cmdproc__hasCommandWaiting(void);
int cmdproc__getCommand(cmdproc_command_t *dest);
