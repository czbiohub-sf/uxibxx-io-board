#include <stdint.h>
#include <string.h>

#include "cmdproc.h"


#define LINE_TERMINATOR '\r'
#define ARG_DELIMITER ','
#define LEFTARGS_START_CH ':'
#define SET_OP_CH '='
#define QUERY_OP_CH '?'
#define IGNORE_CHARS "\n\t "

#define INPUT_BUF_SIZE 32
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
		flags.commandReady = 1;
		}
	else if(inputNBytes >= INPUT_BUF_SIZE) {
		// TODO
		}
	else {
		inputBuffer[inputNBytes++] = ch;
		}
	}

int cmdproc__hasCommandWaiting(void) {
	return flags.commandReady;
	}

int cmdproc__getCommand(cmdproc_command_t *dest) {
	int i;
	int hasLeftArgs = 0;
	int setOp = 0;
	int queryOp = 0;
	int error = 0;
	uint8_t argBuf[ARG_BUF_SIZE];
	int argLen;
	int argIdx;
	int argChIdx;
	for(i = 0; i < inputNBytes; ++i) {
		uint8_t ch = inputBuffer[i];
		if(ch == LEFTARGS_START_CH) {
			hasLeftArgs = 1;
			break;
			}
		else if(ch == SET_OP_CH) {
			setOp = 1;
			break;
			}
		else if(ch == QUERY_OP_CH) {
			queryOp = 1;
			break;
			}
		else if(i >= CMDPROC_MNEM_MAX_LEN) {
			error = 1;
			break;
			}
		dest->mnem[i] = ch;
		}
	dest->mnem[i] = 0;
	argIdx = 0;
	while(!error && hasLeftArgs) {
		if(argIdx >= CMDPROC_MAX_N_LEFTARGS) {
			error = 1;
			break;
			}
		hasLeftArgs = 0;
		argChIdx = 0;
		for(; i < inputNBytes; ++i) {
			uint8_t ch = inputBuffer[i];
			if(ch == ARG_DELIMITER) {
				argIdx++; //FIXME TODO
				argChIdx = 0;
				hasLeftArgs = 1;
				break;
				}
			else if(ch == SET_OP_CH) {
				setOp = 1;
				break;
				}
			else if(ch == QUERY_OP_CH) {
				queryOp = 1;
				break;
				}
			else if(argIdx >= ARG_BUF_SIZE - 1) {
				error = 1;
				break;
				}
			argBuf[argChIdx++] = ch;
			}
		argBuf[argChIdx] = 0;
		dest->leftArgs[argIdx].uintVal = atoi(argBuf);
		}
	if(setOp) {
		dest->cmdType = CMDTYPE_SET;
		}
	else if(queryOp) {
		dest->cmdType = CMDTYPE_QUERY;
		}
	else {
		dest->cmdType = CMDTYPE_DO;
		}
	dest->parseError = error;
	inputNBytes = 0;
	flags.commandReady = 0;
	return 0; // TODO return nonzero if there's an issue
	}