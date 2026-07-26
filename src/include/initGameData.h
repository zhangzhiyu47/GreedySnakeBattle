#ifndef INIT_GAME_DATA_H
#define INIT_GAME_DATA_H

#include "GameAllRunningData.h"

/// Update or initialize for the numberTH food
void foodInit(GameAllRunningData *data, int number);

/// Initialize all the classic-mode game's data
void initGameData(GameAllRunningData *data);

/// Initialize all the unlimit-food-mode game's data
void initGameDataUnlimitFood(GameAllRunningData *data);

#endif // INIT_GAME_DATA_H
