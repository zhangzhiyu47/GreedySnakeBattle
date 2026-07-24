#ifndef PAINTING_H
#define PAINTING_H

#include "Struct/GameAllRunningData.h"
#include "gameConfig.h"

#include <stddef.h>

#define UNLIMIT_FOOD   "#"
#define UNLIMIT_FOOD_C RGB_FG(129, 199, 132)

/// Paint all the walls
void wallPainting(GameAllRunningData *data);

/// Paint game area
void gameAreaPainting(GameAllRunningData const *const data);

/// Draw "↓You" to show which snake the user is controlling.
/// Return 0 for continue game, 1 for exit game interface
int showWhichIsYoursSnake(GameAllRunningData *data);

/// Fill background
void fillBackground(int termW, int termH, void *context);

/// Ask user to zoom in screen or exit app.
/// Return 0 if terminal size is sufficient, 1 to exit game interface
int screenTooSmallPainting(GameAllRunningData *data);

/// Draw the game pause screen
void gamePausePainting(GameAllRunningData *data, const wchar_t *tip);

/// Block until terminal meets minimum size or user presses Q to quit.
/// Return 0 for the size of screen is OK now, 1 for quit app
int screenResizePainting();

#endif
