/**
 * @file gameStartupRelated.h
 * @brief This header declares the functions about launching game.
 */
#ifndef GAME_STARTUP_RELATED_H
#define GAME_STARTUP_RELATED_H

#include "../Struct/GameAllRunningData.h"

/**
 * @brief Initialize all the game's data(All data
 *        within GameAllRunningData).
 *
 * @param[in,out] data All the game's data when the game is running.
 * 
 * @todo
 *      | number | action |
 *      | :----: | :----: |
 *      | 1 | Set the portal |
 *      | 2 | Unlimited food |
 */
void initAllGameData(GameAllRunningData *data);

#endif
