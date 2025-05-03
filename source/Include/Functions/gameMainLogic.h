/**
 * @file gameMainLogic.h
 * @brief This header declares the functions about game main logic.
 */
#ifndef GAME_MAIN_LOGIC_H
#define GAME_MAIN_LOGIC_H

#include "../Struct/GameAllRunningData.h"

/**
 * @brief The main loop of the game process.
 *
 * The main loop of the game process. Move the user
 * and obstacle snake (if he is enabled) (hereinafter
 * referred to as snake for short), execute the
 * printing game interface, the snake eats food,
 * eats the wall, whether it eats itself, judges
 * whether it is dead, quits the game and other game
 * behaviors.
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @bug When the game is running, when the
 *      snake's body is large enough, the
 *      game will get stuck there unless the
 *      signal is sent to stop.
 */
void mainLoopOfGameProcess(GameAllRunningData *data);

#endif
