/**
 * @file food.h
 * @brief This header declares the functions about snake's foods.
 */
#ifndef FOOD_H
#define FOOD_H

#include "../Struct/GameAllRunningData.h"

/**
 * @brief Initialize a food.
 *
 * Update a terminal position (or initialize) for the
 * GameAllRunningData.food[number].
 *
 * @param[in,out] data All the game's data when the game is running.
 * @param number The index of the data.food will be
 *               initialized or updated.
 */
void foodInit(GameAllRunningData *data, int number);

#endif
