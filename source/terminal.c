/**
 * @file terminal.c
 * @brief This source realizes the functions about the terminal.
 */

#include "Include/Compat/snakeFullCompat.h"
#include "Include/Struct/GameConfig.h"
#include "Include/Struct/Point.h"
#include "Include/Functions/standardIO.h"

/**
 * @brief Get the size of the terminal.
 *
 * Get the size of the terminal. In error,
 * both Point.x and Point.y are set to 15.
 *
 * @return Point The size of the terminal.
 */
Point terminalSize() {
    Point termSize = {0};
    struct winsize w = {0};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        termSize.x = w.ws_row;
        termSize.y = w.ws_col;
    } else {
        termSize.x = 15;
        termSize.y = 15;
    }
    return termSize;
}

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
void terminal256ColorTablePainting(const GameConfig *config) {
    clearAll();
    Point termSize=terminalSize();
    int paintingNumberCount=1;
    int MaxNumPaintPerLine=(termSize.y-termSize.y%5)/5;

    for ( int i=0; i<=255; i++,paintingNumberCount++ ) {
        printf("\033[48;5;%dm",i);
        printf("\033[38;5;%dm %3d ",i==15 || i==231 || i==255 || i==254?0:15,i);
        if ( paintingNumberCount==MaxNumPaintPerLine ) {
            printf("\n");
            paintingNumberCount=0;
        }
    }
    
    printf("\033[48;5;%dm",config->scrnColr);
    printf("\033[38;5;%dm",config->wordColr);
    blockWaitUserEnter();
}
