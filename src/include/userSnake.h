/**
 * @file userSnake.h
 * @brief This header declares the functions about snake's snake.
 */
#ifndef USER_SNAKE_H
#define USER_SNAKE_H

#include "Struct/GameAllRunningData.h"

#include <stdbool.h>

/// Move user's snake, go through walls, come out on the other side
void userSnakeMoveCross(GameAllRunningData *data);

/// Move user's snake
void userSnakeMove(GameAllRunningData *data);

/**
 * @brief Control the direction of user's snake movement.
 *
 * | input | action |
 * | :---: | :----: |
 * | ←/A/7 | ← |
 * | ↑/W/5 | ↑ |
 * | ↓/S/8 | ↓ |
 * | →/D/9 | → |
 * | j/J | Jump |
 * | f/F/Tab | Fly |
 * | p/P | Pause |
 * | r/R | Repaint |
 * | Esc | Block game |
 * | o/O | Game over |
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @return The state of the game when the game is running.
 * @retval 0 Game will be last running.
 * @retval 1 Game will be over.
 */
int userSnakeMoveDirecControl(GameAllRunningData *data);

/// User's snake eat foods or death the obstacle snake's body
void userSnakeEatFood(GameAllRunningData *data);

/// User's snake eat unlimited foods
void userSnakeEatUnlimitedFood(GameAllRunningData *data);

/// Determine if there's unlimited food at the coordinates
bool isUnlimitedFoodAt(uint64_t x, uint64_t y);

/**
 * @brief Is user's snake eating its own body.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating its own body.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatSelf(const GameAllRunningData *data);

/**
 * @brief Is user's snake eating wall.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating wall.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatWall(const GameAllRunningData *data);

/**
 * @brief Is user's snake eating obstacle snake body.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating obstacle snake body.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatObsSnake(const GameAllRunningData *data);

#endif
