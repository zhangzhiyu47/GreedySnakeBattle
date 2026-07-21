#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "Struct/GameAllRunningData.h"
#include "gameConfig.h"
#include <unistd.h>

/// Show game menu.
bool showGameMenu(GameConfig *config);

/// Introduce and explain the game in detail.
void gameIntroduction(const char *button);

/// Request the user to agree to the End User License Agreement
int requestUserAgreeEULA();

/// Ask user whether to check app errors and display logs
void showErrorLog();

#endif // GAME_MENU_H
