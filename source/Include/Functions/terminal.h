/**
 * @file terminal.h
 * @brief This header declares all the functions about the terminal.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "../Struct/GameConfig.h"
#include "../Struct/Point.h"

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
 * @param[in] config Game configuration of program reading
 *                       and writing configuration files.
 */
void terminal256ColorTablePainting(const GameConfig *config);

#endif
