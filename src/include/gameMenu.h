#ifndef GAME_MENU_H
#define GAME_MENU_H

#include "GameAllRunningData.h"
#include "gameConfig.h"
#include "initGameData.h"

/// Show game menu.
GameMode showGameMenu(GameConfig *config);

/// Introduce and explain the game in detail.
void gameIntroduction(const char *button);

/// Request the user to agree to the End User License Agreement
int requestUserAgreeEULA(const char *tip);

/// Ask user whether to check app errors and display logs
void showErrorLog();

/// Show new version update information
void showNewVersionInfo();

#endif // GAME_MENU_H
