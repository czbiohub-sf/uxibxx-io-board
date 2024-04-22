#define CMDPROC_MNEM_MAX_LEN 16
#define CMDPROC_MAX_N_LEFTARGS 1
#define CMDPROC_MAX_N_RIGHTARGS 1

typedef enum {
	CMDTYPE_DO = 1,
	CMDTYPE_QUERY = 2,
	CMDTYPE_SET = 3
	} cmdproc_cmdtype_t;

typedef union {
	uint16_t uintVal;
	} cmdproc_arg_t;

typedef struct {
	cmdproc_cmdtype_t cmdType;
	char mnem[CMDPROC_MNEM_MAX_LEN + 1];
	int nLeftArgs;
	int nRightArgs;
	cmdproc_arg_t leftArgs[CMDPROC_MAX_N_LEFTARGS];
	cmdproc_arg_t rightArgs[CMDPROC_MAX_N_LEFTARGS];
	int parseError;
	} cmdproc_command_t;
