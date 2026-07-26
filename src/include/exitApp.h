#ifndef EXIT_APP_H
#define EXIT_APP_H

#include "GameAllRunningData.h"

#define EXIT_NORMAL 0  //< App exit normally
#define EXIT_ERROR 1   //< App exit because of error

/// Exit the application
void exitApp(int retn, const char *tip, const GameAllRunningData *data);

#endif
