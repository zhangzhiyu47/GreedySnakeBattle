#ifndef INIT_GAME_DATA_H
#define INIT_GAME_DATA_H

#include "GameAllRunningData.h"

typedef enum GameMode {
    MODE_QUIT,
    MODE_CLASSIC,
    MODE_UNLIMITED_FOOD,
} GameMode;

/// Update or initialize for the numberTH food
void foodInit(GameAllRunningData *data, int number);

/// Initialize all the classic-mode game's data
void initGameData(GameAllRunningData *data, GameMode mode);

#endif // INIT_GAME_DATA_H
