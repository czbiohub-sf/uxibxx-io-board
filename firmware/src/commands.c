#include <avr/pgmspace.h>

#include "cmdproc.h"


const cmdproc_cmd_spec_t cmdproc__commandSpecs[] PROGMEM = {
	{
		.cmdType = CMDTYPE_QUERY,
		.mnem="IDN",
		.nLeftArgs=0,
		.nRightArgs=0,
		},
	{
		.cmdType = CMDTYPE_DO,
		.mnem="DFU",
		.nLeftArgs=0,
		.nRightArgs=0,
		},
	{
		.cmdType = CMDTYPE_DO,
		.mnem="RST",
		.nLeftArgs=0,
		.nRightArgs=0,
		},
	{
		.cmdType = CMDTYPE_QUERY,
		.mnem="TLS",
		.nLeftArgs=0,
		.nRightArgs=0,
		},
	{
		.cmdType = CMDTYPE_QUERY,
		.mnem="TCP",
		.nLeftArgs=1,
		.nRightArgs=0,
		.leftArgTypes={ARGTYPE_UINT8},
		},
	{
		.cmdType = CMDTYPE_QUERY,
		.mnem="INP",
		.nLeftArgs=1,
		.nRightArgs=0,
		.leftArgTypes={ARGTYPE_UINT8},
		},
	{
		.cmdType = CMDTYPE_QUERY,
		.mnem="OUT",
		.nLeftArgs=1, 
		.nRightArgs=0,
		.leftArgTypes={ARGTYPE_UINT8},
		},
	{
		.cmdType = CMDTYPE_SET,
		.mnem="OUT",
		.nLeftArgs=1, 
		.nRightArgs=1,
		.leftArgTypes={ARGTYPE_UINT8},
		.rightArgTypes={ARGTYPE_UINT8}
		},
	{
		.cmdType = CMDTYPE_QUERY,
		.mnem="DIR",
		.nLeftArgs=1, 
		.nRightArgs=0,
		.leftArgTypes={ARGTYPE_UINT8},
		},
	{
		.cmdType = CMDTYPE_SET,
		.mnem="DIR",
		.nLeftArgs=1, 
		.nRightArgs=1,
		.leftArgTypes={ARGTYPE_UINT8},
		.rightArgTypes={ARGTYPE_UINT8}
		},
	};

const int cmdproc__commandSpecsLen = sizeof(cmdproc__commandSpecs) / sizeof(cmdproc__commandSpecs[0]);