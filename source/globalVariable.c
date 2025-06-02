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
 * @brief Apply the written log file name.
 */
char logFile[2048] = {0};

/**
 * @brief Configuration information file name of the game.
 */
char configFile[2048] = {0};

/**
 * @brief Directory where game settings are located.
 */
char configDir[1024] = {0};
