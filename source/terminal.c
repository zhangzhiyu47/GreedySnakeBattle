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
void terminal256ColorTablePainting(const GameConfig* config) {
    const int colorGroup[] = {
        16, 17, 18, 19, 20, 21,
        52, 53, 54, 55, 56, 57,
        88, 89, 90, 91, 92, 93,
        124, 125, 126, 127, 128, 129,
        160, 161, 162, 163, 164, 165,
        196, 197, 198, 199, 200, 201,
    };
    
    clearScreen();

    for (int i = 0; i <= 15; ++i) {
        if (i == 0) {
            printf("标准颜色\n");
            printf("\033[38;5;%dm", 15);
        } else if (i == 8) {
            printf("\033[48;5;%dm", config->scrnColr);
            printf("\033[38;5;%dm", config->wordColr);
            printf("\n\n高强度颜色\n");
            printf("\033[38;5;%dm", 0);
        }
        printf("\033[48;5;%dm %3d ", i, i);
    }

    printf("\033[48;5;%dm", config->scrnColr);
    printf("\033[38;5;%dm", config->wordColr);
    printf("\n\n216种颜色\n");

    for (int i = 0; i < 6; ++i) {
        printf("\033[48;5;%dm", config->scrnColr);
        printf("\033[38;5;%dm", config->wordColr);
        printf("\n第%d颜色块组\n", i + 1);

        if (i <= 4) {
            printf("\033[38;5;15m");
        } else {
            printf("\033[38;5;0m");
        }

        for (int j = 0; j < 36; ++j) {
            printf("\033[48;5;%dm %3d ", colorGroup[j] + i * 6, colorGroup[j] + i * 6);
            if ((j + 1) % 6 == 0) {
                printf("\033[48;5;%dm\n", config->scrnColr);
            }
        }
    }
    
    for (int i = 232; i <= 255; ++i) {
        if (i == 232) {
            printf("\n\n深灰度颜色\n");
            printf("\033[38;5;%dm", 15);
        } else if (i == 244) {
            printf("\033[48;5;%dm", config->scrnColr);
            printf("\033[38;5;%dm", config->wordColr);
            printf("\n\n浅灰度颜色\n");
            printf("\033[38;5;%dm", 0);
        }
        printf("\033[48;5;%dm %3d ", i, i);
    }

    printf("\033[48;5;%dm", config->scrnColr);
    printf("\033[38;5;%dm", config->wordColr);
    putchar('\n');
    blockWaitUserEnter();
}
