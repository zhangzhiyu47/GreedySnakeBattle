/**
 * @file globalVariable.h
 * @brief This header declares all global variables.
 */
#ifndef GLOBAL_VARIABLE_H
#define GLOBAL_VARIABLE_H

#include "../Struct/Point.h"
#include "../Struct/GameConfig.h"

#include <unistd.h>

/**
 * @brief Check if the configuration file failed to open.
 */
extern bool isConfigFileOpenFail;

/**
 * @brief The height(HIGH) and width(WIDE) of the game interface.
 */
extern int HIGH,WIDE;

/**
 * @brief The game config for outline mode.
 */
extern GameConfig outlineModeConfig;

/**
 * @brief Original terminal settings.
 */
extern struct termios originalTermios;

/**
 * @brief Apply the written log file name.
 */
extern const char* logFile;

#endif
