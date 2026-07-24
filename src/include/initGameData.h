#ifndef INIT_GAME_DATA_H
#define INIT_GAME_DATA_H

#include "Struct/GameAllRunningData.h"

/// Initialize all the classic-mode game's data
/// TODO: Set the portal
void initGameData(GameAllRunningData *data);

/// Initialize all the unlimit-food-mode game's data
void initGameDataUnlimitFood(GameAllRunningData *data);

#endif // INIT_GAME_DATA_H
