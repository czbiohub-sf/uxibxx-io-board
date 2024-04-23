enum gpio_terminal_dir {
	DIR_IN = 0,
	DIR_OUT = 1,
	};


extern const int gpio__nTerminals;


void gpio__init(void);
int gpio__getTerminalNo(int terminalIdx);
int gpio__getInput(int terminalNo);
int gpio__getOutput(int terminalNo);
int gpio__setOutput(int terminalNo, int on);
int gpio__setDirection(int terminalNo, enum gpio_terminal_dir dir);
int gpio__getDirection(int terminalNo);
int gpio__supportsInput(int terminalNo);
int gpio__supportsOutput(int terminalNo);
