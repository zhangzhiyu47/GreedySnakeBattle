/**
 * @file globalVariable.c
 * @brief This source realizes the global variable.
 */

#include <stdbool.h>
#include "Include/GlobalVariable/globalVariable.h"

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
