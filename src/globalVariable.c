#include "include/GlobalVariable/globalVariable.h"
#include <stdatomic.h>
#include <stdint.h>
#include <termios.h>

uint64_t HIGH=20,WIDE=61;

struct termios originalTermios;

char configDir[1024] = {0};

char configFile[2048] = {0};
char logFile[2048] = {0};
char errSignFile[2048] = {0};
char lockFile[2048] = {0};

int lockFileFd = 0;

atomic_bool needRedraw = ATOMIC_VAR_INIT(false);
