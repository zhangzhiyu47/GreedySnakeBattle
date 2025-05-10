/**
 * @file painting.h
 * @brief This header declares the functions about painting interface.
 */

#ifndef PAINTING_H
#define PAINTING_H

#include "../Struct/GameAllRunningData.h"

/**
 * @brief Paint all the walls.
 * 
 * Use @ref HIGH,@ref WIDE,GameAllRunningData.wallNum
 * and GameAllRunningData.wall to paint the outer fences
 * and obstacle walls.
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @attention The parameters are not checked, the wrong parameters
 *            will produce unpredictable results or crash.
 */
void wallPainting(GameAllRunningData *data);

/**
 * @brief Paint the game interface when the game is running.
 *
 * Paint foods,user's snake,obstacle snake(if it is enable)
 * and user's score.
 *
 * @param[in] data All the game's data when the game is running.
 */
void gameInterfacePainting(GameAllRunningData const *const data);

/**
 * @brief Paint the game-over interface and update the highest
 *        history score(If the configuration file open failed,
 *        it will start the Offline Mode).
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void gameOverPainting(GameAllRunningData *data);

#endif
