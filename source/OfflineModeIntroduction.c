/**
 * @file OfflineModeIntroduction.c
 * @brief This file introduces the offline mode
 *
 * This file only introduces offline mode and has no actual
 * functionality, no need to compile connections.
 */

/**
 * @page OffLineMode OffLine mode introduction
 *
 * The mode is opened when the configuration file cannot be
 * opened. There are many places to read the configuration file,
 * and there are many reasons why it cannot be read. Once the
 * offline mode is opened, it cannot be closed before the game is
 * over. In this mode, the game no longer reads the configuration
 * file, but relies on its own configuration variables and default
 * configuration to play the game.
 *
 * @attention In this mode, the game settings can still be adjusted,
 *            but in the configuration file that will not be saved,
 *            the settings can only last until the end of the game,
 *            and it is impossible to automatically restore the
 *            content set by the player when the game is restarted.
 *            Changing the game settings in this mode is not recommended.
 */
