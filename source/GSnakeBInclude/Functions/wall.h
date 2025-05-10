/**
 * @file wall.h
 * @brief This header declares the functions about obstacle walls.
 */
#ifndef WALL_H
#define WALL_H

#include "../Struct/GameAllRunningData.h"

/**
 * @brief Initialize all the obstacle walls.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void wallInit(GameAllRunningData *data);

#endif
