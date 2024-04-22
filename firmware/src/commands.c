#include "cmdproc.h"


enum commands {
	COMMAND_INP_QUERY,
	COMMAND_OUT_QUERY,
	COMMAND_OUT_SET,
	COMMAND_DIR_QUERY,
	COMMAND_DIR_SET,
	};

cmdproc_cmd_spec_t cmdproc_commands[] = {
	{
		.cmdId = COMMAND_INP_QUERY,
		.cmdType = CMDTYPE_QUERY,
		.mnem="INP",
		.nLeftArgs=1,
		.nRightArgs=0,
		.leftArgTypes={ARGTYPE_UINT8},
		},
	{
		.cmdId = COMMAND_OUT_QUERY,
		.cmdType = CMDTYPE_QUERY,
		.mnem="OUT",
		.nLeftArgs=1, 
		.nRightArgs=0,
		.leftArgTypes={ARGTYPE_UINT8},
		},
	{
		.cmdId = COMMAND_OUT_SET,
		.cmdType = CMDTYPE_SET,
		.mnem="OUT",
		.nLeftArgs=1, 
		.nRightArgs=1,
		.leftArgTypes={ARGTYPE_UINT8},
		.rightArgTypes={ARGTYPE_UINT8}
		},
	{
		.cmdId = COMMAND_DIR_QUERY,
		.cmdType = CMDTYPE_QUERY,
		.mnem="DIR",
		.nLeftArgs=1, 
		.nRightArgs=0,
		.leftArgTypes={ARGTYPE_UINT8},
		},
	{
		.cmdId = COMMAND_DIR_SET,
		.cmdType = CMDTYPE_SET,
		.mnem="DIR",
		.nLeftArgs=1, 
		.nRightArgs=1,
		.leftArgTypes={ARGTYPE_UINT8},
		.rightArgTypes={ARGTYPE_UINT8}
		},
	};
