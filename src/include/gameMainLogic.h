#ifndef GAME_MAIN_LOGIC_H
#define GAME_MAIN_LOGIC_H

#include "Struct/GameAllRunningData.h"

/**
 * @brief The main logic of the game
 *
 * @bug If snake's body is large enough, the
 *      game will get stuck there unless the
 *      signal is sent to stop.
 */
void gameMainLogic(GameAllRunningData *data);

#endif
