#include "include/GlobalVariable/globalVariable.h"
#include "include/Struct/Point.h"
#include "include/Functions/terminal.h"
#include "include/logger.h"
#include "include/exitApp.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

/**
 * @brief Clear the screen display content.
 */
void clearScreen() {
    printf("\033[H\033[2J\033[3J");
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

/// Get the size of the terminal
Point terminalSize() {
    Point termSize = {0};
    struct winsize w = {0};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        termSize.x = w.ws_col;
        termSize.y = w.ws_row;
    } else {
        logger(LOG_ERROR, "ioctl: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错", NULL);
    }
    return termSize;
}

void setBackgroundColor() {
    printf(RGB_BG(255, 250, 240));
}

void setWordColor() {
    printf(RGB_FG(45, 45, 45));
}

void resetColor() {
    printf("\033[0m");
    setBackgroundColor();
    setWordColor();
}
