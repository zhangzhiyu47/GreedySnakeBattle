#ifndef PAINTING_H
#define PAINTING_H

#include "../Struct/GameAllRunningData.h"
#include "../gameConfig.h"

#include <stddef.h>

/// Paint all the walls
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

/// Draw "↓You" to show which snake the user is controlling
void showWhichIsYoursSnake(GameAllRunningData *data);

/// Fill background
void fillBackground(int termW, int termH, void *context);

/// Ask user to zoom in screen or exit app
/// Return 0 if terminal size is sufficient, 1 to exit game interface
int screenTooSmallPainting(GameAllRunningData *data);

/// Draw the game pause screen
void gamePausePainting(GameAllRunningData *data);

#endif
