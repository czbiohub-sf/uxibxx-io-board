#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cmdproc.h"


#define LINE_TERMINATOR '\r'
#define ARG_DELIMITER ','
#define LEFTARGS_START_CH ':'
#define SET_OP_CH '='
#define QUERY_OP_CH '?'
#define IGNORE_CHARS "\n\t "

#define INPUT_BUF_SIZE 33
#define ARG_BUF_SIZE 17


static uint8_t inputNBytes;
static uint8_t inputBuffer[INPUT_BUF_SIZE];
static struct {
	unsigned int commandReady :1;
	} flags;


void cmdproc__init(void) {
	inputNBytes = 0;
	flags.commandReady = 0;
	}

void cmdproc__processIncomingChar(uint8_t ch) {
	if(!ch || strchr(IGNORE_CHARS, ch) || flags.commandReady)
		return;
	else if(ch == LINE_TERMINATOR) {
		inputBuffer[inputNBytes] = 0;
		flags.commandReady = 1;
		}
	else if(inputNBytes >= INPUT_BUF_SIZE - 1) {
		// TODO maybe set an error flag or something?
		}
	else {
		inputBuffer[inputNBytes++] = ch;
		}
	}

int cmdproc__hasCommandWaiting(void) {
	return flags.commandReady;
	}

int parseArgVal(
		cmdproc_argval_t *dest, const char *buf, cmdproc_argtype_t argType) {
	int scanfResult;
	unsigned int uintVal;
	switch(argType) {
		case ARGTYPE_UINT8:
		case ARGTYPE_UINT16:
			scanfResult = sscanf((char *)buf, "%u", &uintVal);
			if(scanfResult != 1)
				return -1;
				// TODO maybe have distinct error values for different problems	
			break;
		case ARGTYPE_STRING:
			strncpy(dest->stringVal, buf, CMDPROC_ARG_MAX_LEN);
			dest->stringVal[CMDPROC_ARG_MAX_LEN] = 0;
			return 0;
		default:
			return -1;		
		}
	switch(argType) {
		case ARGTYPE_UINT8:
			if(uintVal > 255)
				return -1;
			dest->uint8Val = (uint8_t) uintVal;
			return 0;
		case ARGTYPE_UINT16:
			dest->uint16Val = (uint16_t) uintVal;
			return 0;
		default:
			break;
		}
	return -1;
	}

int getCommandSpec(
		cmdproc_cmd_spec_t *dest, const char *mnem, cmdproc_cmdtype_t cmdType
		) {
	for(int i = 0; i < cmdproc__commandSpecsLen; ++i) {
		memcpy_P(
			dest,
			&cmdproc__commandSpecs[i],
			sizeof(cmdproc_cmd_spec_t)
			);
		if(
				!strncmp(mnem, dest->mnem, CMDPROC_MNEM_MAX_LEN)
				&& dest->cmdType == cmdType
				)
			return i;
		}
	return -1;
	}

cmdproc_error_t parseArgs(char *str, cmdproc_argval_t *destArgs,
	                      const cmdproc_argtype_t *argTypes, int nArgs) {
	char argBuf[CMDPROC_ARG_MAX_LEN + 1] = {[CMDPROC_ARG_MAX_LEN] = 0};
	int argIdx = 0;
	char *argStart = str;
	char *nextArgStart = str;
	char *argEnd;
	while(*argStart) {
		if(argIdx >= nArgs)
			return ERROR_N_ARGS;
		if((argEnd = strchr(argStart, ARG_DELIMITER))) {
			*argEnd = 0;
			nextArgStart = argEnd + 1;
			}
		else {
			nextArgStart = NULL;
			}
		if(strlen(argStart) > CMDPROC_ARG_MAX_LEN)
			return ERROR_ARG_FMT;
		strncpy(argBuf, argStart, CMDPROC_ARG_MAX_LEN);
		// TODO we don't actually need to do that copy,
		// could enforce length in parseArgVal()
		if(parseArgVal(&destArgs[argIdx], argBuf, argTypes[argIdx]))
			return ERROR_ARG_FMT;
		argIdx++;
		if(!nextArgStart)
			break;
		argStart = nextArgStart;
		}
	if(argIdx != nArgs)
		return ERROR_N_ARGS;
	return 0;
	}

cmdproc_error_t cmdproc__getCommand(cmdproc_command_t *dest) {
	int error = 0;
	char leftBuf[INPUT_BUF_SIZE] = {0};
	char rightBuf[INPUT_BUF_SIZE] = {0};
	char *leftEnd;
	char *mnemEnd;
	cmdproc_cmd_spec_t cmdSpec;
	int haveLeftArgs = 0;


	if((leftEnd = strchr((char *)inputBuffer, QUERY_OP_CH))) {
		dest->cmdType = CMDTYPE_QUERY;
		}
	else if((leftEnd = strchr((char *)inputBuffer, SET_OP_CH))) {
		dest->cmdType = CMDTYPE_SET;
		}
	else {
		dest->cmdType = CMDTYPE_DO;
		leftEnd = (char *)&inputBuffer[inputNBytes];
		}
	*leftEnd = 0;

	strcpy(leftBuf, (char *)inputBuffer);
	if(leftEnd < (char *)&inputBuffer[inputNBytes]) {
		strcpy(rightBuf, leftEnd + 1);
		}
	if((mnemEnd = strchr(leftBuf, LEFTARGS_START_CH))) {
		haveLeftArgs = 1;
		}
	else {
		mnemEnd = &leftBuf[strlen(leftBuf)];
		}
	*mnemEnd = 0;
	if(strlen(leftBuf) > CMDPROC_MNEM_MAX_LEN)
		error = ERROR_CMD;
	else {
		strncpy(dest->mnem, leftBuf, CMDPROC_MNEM_MAX_LEN);
		if(getCommandSpec(&cmdSpec, dest->mnem, dest->cmdType) < 0)
			error = ERROR_CMD;
		}

	if(!error) {
		if(haveLeftArgs) {
			error = parseArgs(
				mnemEnd + 1,
				dest->leftArgs,
				cmdSpec.leftArgTypes,
				cmdSpec.nLeftArgs
				);
			}
		else if(cmdSpec.nLeftArgs)
			error = ERROR_N_ARGS;
		}

	if(!error) {
		if(strlen(rightBuf)){
			error = parseArgs(
				rightBuf,
				dest->rightArgs,
				cmdSpec.rightArgTypes,
				cmdSpec.nRightArgs
				);
			}
		else if(cmdSpec.nRightArgs)
			error = ERROR_N_ARGS;
		}

	dest->parseError = error;
	inputNBytes = 0;
	flags.commandReady = 0;
	return error;
	}
