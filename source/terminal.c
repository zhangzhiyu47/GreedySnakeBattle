/**
 * @file terminal.c
 * @brief This source realizes the functions about the terminal.
 */

#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Struct/GameConfig.h"
#include "GSnakeBInclude/Struct/Point.h"
#include "GSnakeBInclude/Functions/standardIO.h"
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Clear the screen display content.
 */
void clearScreen() {
    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #elif defined(__unix__) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
        system("clear");
    #else
        printf("\033[H\033[J");
    #endif
}

/**
 * @brief Initialize terminal settings
 */
void initTerminalSettings() {
    tcgetattr(STDIN_FILENO, &originalTermios);

    struct termios newt = originalTermios;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

/**
 * @brief Restore original terminal settings
 */
void restoreTerminalSettings() {
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
}

/**
 * @brief Get the size of the terminal.
 *
 * Get the size of the terminal. In error,
 * both Point.x and Point.y are set to 24.
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
        termSize.x = 24;
        termSize.y = 24;
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
    clearScreen();
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
