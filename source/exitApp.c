/**
 * @file exitApp.c
 * @brief This source realizes the @ref exitAll function.
 */

#include "Include/Struct/GameAllRunningData.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Exit the application.
 * @param retn The exiting number.
 * @param data The all game data when the game is running.
 */
void exitApp(int retn,const GameAllRunningData *data) {
    printf("\n游戏结束\n");
    printf("\033[?25h\033[0m");
    fflush(stdout);
    
    free((void*)data);
    
    printf("\033[?25h");
    
    exit(retn);
}
