#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

#include "../Struct/Point.h"
#include "../gameConfig.h"

#include <stdint.h>
#include <stdatomic.h>

/**
 * @brief The height(HIGH) and width(WIDE) of the game interface.
 */
extern uint64_t HIGH,WIDE;

/**
 * @brief Original terminal settings.
 */
extern struct termios originalTermios;

/**
 * @brief Apply the written log file name.
 */
extern char logFile[2048];

/// Record application error flag.
extern char errSignFile[2048];

/// Application singleton lock file.
extern char lockFile[2048];

/**
 * @brief Configuration information file name of the game.
 */
extern char configFile[2048];

/**
 * @brief Directory where game settings are located.
 */
extern char configDir[1024];

//< Whether to redraw
extern atomic_bool needRedraw;

/// Application singleton lock file descriptor
extern int lockFileFd;

#endif
