/**
 * @file terminal.h
 * @brief This header declares all the functions about the terminal.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "../gameConfig.h"
#include "../Struct/Point.h"

#define RGB_BG(r, g, b)  "\033[48;2;" #r ";" #g ";" #b "m"
#define RGB_FG(r, g, b)  "\033[38;2;" #r ";" #g ";" #b "m"

/// Set terminal background color
void setBackgroundColor();

/// Set terminal word color
void setWordColor();

/**
 * Reset color
 * Clear color settings, then set background and foreground color
 */
void resetColor();

/// Clear the screen display content
void clearScreen();

/// Initialize terminal settings
void initTerminalSettings();

/// Restore original terminal settings
void restoreTerminalSettings();

/**
 * @brief Get the size of the terminal.
 *
 * Get the size of the terminal. In error,
 * both Point.x and Point.y are set to -1.
 *
 * @return Point The size of the terminal.
 */
Point terminalSize();

#endif
