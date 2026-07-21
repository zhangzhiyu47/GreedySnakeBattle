#ifndef INITIALIZE_APP_H
#define INITIALIZE_APP_H

#include "gameConfig.h"

/// Initialize configuration file and render background
int initializeApp(GameConfig *config);

/**
 * @brief Creates application directories including
 *        config and log directories.
 *
 * Follows XDG Base Directory Specification:
 * - Uses $XDG_CONFIG_HOME/gsb/ if set
 * - Falls back to ~/.config/gsb/ otherwise
 */
void createAppDirectories();

/// Check the application singleton lock file
void checkLockFile();

#endif // INITIALIZE_APP_H
