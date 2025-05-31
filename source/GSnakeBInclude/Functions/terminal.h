/**
 * @file terminal.h
 * @brief This header declares all the functions about the terminal.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "../Struct/GameConfig.h"
#include "../Struct/Point.h"

/**
 * @brief Clear the screen display content.
 */
void clearScreen();

/**
 * @brief Initialize terminal settings
 */
void initTerminalSettings();

/**
 * @brief Restore original terminal settings
 */
void restoreTerminalSettings();

/**
 * @brief Get the size of the terminal.
 *
 * Get the size of the terminal. In error,
 * both Point.x and Point.y are set to 15.
 *
 * @return Point The size of the terminal.
 */
Point terminalSize();

/**
 * @brief Paint the terminal 256 color table.
 *
 * Support terminal 256-color printing of "true color"
 * graphics cards with 16-to 24-bit colors (common Xterm,
 * Konsole of KDE, and all terminals based on libvte
 * (including GNOM)) for interface color setting and
 * terminal support color viewing.
 *
 * The printed color is divided into three parts.
 *
 * 1. Standard color
 *    1. Standard
 *    2. Highlight
 *
 * 2. 216 colors
 *    It is divided into six color block groups, and the color
 *    system of the color in the corresponding position of each
 *    group is the same, and the color from group 1 to group 6
 *    is getting lighter and lighter.
 *
 * 3. Gray scale color
 *    1. Dark grey
 *    2. Light grey
 * 
 * @param[in] config Game configuration of program reading
 *                       and writing configuration files.
 */
void terminal256ColorTablePainting(const GameConfig *config);

#endif
