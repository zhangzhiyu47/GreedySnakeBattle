#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "Struct/GameAllRunningData.h"
#include "gameConfig.h"
#include <unistd.h>

#define GAME_MODE_QUIT 0
#define GAME_MODE_CLASSIC 1
#define GAME_MODE_UNLIMIT_FOOD 2

/// Show game menu.
int showGameMenu(GameConfig *config);

/// Introduce and explain the game in detail.
void gameIntroduction(const char *button);

/// Request the user to agree to the End User License Agreement
int requestUserAgreeEULA(const char *tip);

/// Ask user whether to check app errors and display logs
void showErrorLog();

/// Show new version update information
void showNewVersionInfo();

#endif // GAME_MENU_H
