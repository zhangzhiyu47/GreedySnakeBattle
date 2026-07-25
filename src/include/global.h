#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

#include "Struct/Point.h"
#include "gameConfig.h"

#include <stdint.h>
#include <stdatomic.h>

/// The height(HIGH) and width(WIDE) of the game area
extern uint64_t HIGH,WIDE;

/// Original terminal settings
extern struct termios originalTermios;

/// Log file
extern char logFile[2048];

/// Record application error flag
extern char errSignFile[2048];

/// Application singleton lock file
extern char lockFile[2048];

/// New version update flag
extern char updateSignFile[2048];

/// EULA update flag
extern char EULAUpdateSign[2048];

/// Configuration information file
extern char configFile[2048];

/// Directory where game settings are located.
extern char configDir[1024];

/// Whether to redraw
extern atomic_bool needRedraw;

/// Application singleton lock file descriptor
extern int lockFileFd;

#endif
