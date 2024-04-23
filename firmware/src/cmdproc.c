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
				return -1; // TODO maybe have distinct error values for different problems	
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

int cmdproc__getCommand(cmdproc_command_t *dest) {
	int error = 0;
	char leftBuf[INPUT_BUF_SIZE] = {0};
	char rightBuf[INPUT_BUF_SIZE] = {0};
	char argBuf[CMDPROC_ARG_MAX_LEN + 1] = {0};
	char *leftEnd;
	char *mnemEnd;
	char *argEnd;
	char *leftArgStart;
	char *nextLeftArgStart;
	int leftArgIdx = 0;
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
	// TODO: process the mnem earlier, look up the command spec, parse args according to specified type
	if((mnemEnd = strchr(leftBuf, LEFTARGS_START_CH))) {
		leftArgStart = mnemEnd + 1;
		while(*leftArgStart) {
			if(leftArgIdx >= CMDPROC_MAX_N_LEFTARGS) {
				error |= 8;
				break;
				}
			if((argEnd = strchr(leftArgStart, ARG_DELIMITER))) {
				*argEnd = 0;
				nextLeftArgStart = argEnd + 1;
				}
			else {
				nextLeftArgStart = NULL;
				}
			if(strlen(leftArgStart) > CMDPROC_ARG_MAX_LEN)
				error |= 4;
			strncpy(argBuf, leftArgStart, CMDPROC_ARG_MAX_LEN);
			error |= 2 * !!parseArgVal(
				&dest->leftArgs[leftArgIdx], argBuf, ARGTYPE_UINT16); // TODO actually use command specs
			leftArgIdx++;
			leftArgStart = nextLeftArgStart;
			}
		}
	else {
		mnemEnd = &leftBuf[strlen(leftBuf)];
		}
	*mnemEnd = 0;
	if(strlen(leftBuf) > CMDPROC_MNEM_MAX_LEN)
		error |= 16;
	strncpy(dest->mnem, leftBuf, CMDPROC_MNEM_MAX_LEN);

	//temp TODO
	if(strlen(rightBuf)) {
		error |= 32 * !!parseArgVal(&dest->rightArgs[0], rightBuf, ARGTYPE_UINT16);
		}

	dest->parseError = error;
	inputNBytes = 0;
	flags.commandReady = 0;
	return error;
	}