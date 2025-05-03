/**
 * @file exitApp.h
 * @brief This headr declares the function @ref exitApp.
 */
#ifndef EXIT_APP_H
#define EXIT_APP_H

#include "../Struct/GameAllRunningData.h"

/**
 * @brief Exit the application.
 * @param retn The exiting number.
 * @param data The all game data when the game is running.
 */
void exitApp(int retn,const GameAllRunningData *data);

#endif
