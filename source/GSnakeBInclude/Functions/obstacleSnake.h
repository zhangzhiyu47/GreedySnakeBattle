/**
 * @file obstacleSnake.h
 * @brief This header declares the functions about obstacle snake.
 */
#ifndef ONSTACLE_SNAKE_H
#define ONSTACLE_SNAKE_H

#include "../Struct/GameAllRunningData.h"

/**
 * @brief Initialize the obstacle snake if it is enable.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsInit(GameAllRunningData *data);

/**
 * @brief Control the moving direction of obstacle snake.
 *
 * Control the moving direction of the obstacle snake
 * and move to the nearest food.
 *
 * @param[in,out] data All the game's data when the game is running.
 * @todo Unable to automatically avoid user snakes and
 *       walls, you need to add judgment code.
 */
void obsMoveDirecControl(GameAllRunningData *data);

/**
 * @brief The position of the moving obstacle snake.
 *
 * The position of the obstacle's last body is copied from
 * the previous body, and the coordinates of the obstacle
 * head are added to GameAllRunningData.obsSnkNxtXDrc and
 * GameAllRunningData.obsSnkNxtYDrc.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsMove(GameAllRunningData *data);

/**
 * @brief Obstacle snake eat foods.
 * @param[in,out] data All the game's data when the game is running.
 */
void obsEatFood(GameAllRunningData *data);

/**
 * @brief Judge whether the obstacle snake eats the wall
 *        or the user snake.
 *
 * Judge whether the obstacle snake eats the wall or the
 * user snake. If obstacle snake eat them,it will change
 * the status(GameAllRunningData.obsState) of the obstacle
 * snake(to 1 or 2, see GameAllRunningData.obsState for
 * specific meaning) to describe the situation of obstacle.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsEatWallsOrUserSnake(GameAllRunningData *data);

#endif
