/**
 * @file globalVariable.c
 * @brief This source realizes the global variable.
 */

#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include <termios.h>
#include <stdbool.h>
#include <unistd.h>

/**
 * @brief Check if the configuration file failed to open.
 */
bool isConfigFileOpenFail=false;

/**
 * @brief The height(HIGH) and width(WIDE) of the game interface.
 */
int HIGH=20,WIDE=61;

/**
 * @brief The game config for outline mode.
 */
GameConfig outlineModeConfig={0};

/**
 * @brief Original terminal settings.
 */
struct termios originalTermios;

/**
 * @brief Record the pid of the child process.
 */
pid_t childPid;
