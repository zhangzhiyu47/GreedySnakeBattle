#include "include/global.h"
#include "include/Point.h"
#include "include/terminal.h"
#include "include/logger.h"
#include "include/exitApp.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

void clearScreen() {
    printf("\033[H\033[2J\033[3J");
}

void initTerminalSettings() {
    printf("\033[?1049h");
    printf("\033[?1000;1002;1006h\033[?25l");
    printf("\033]0;Greedy Snake Battle\x07");

    tcgetattr(STDIN_FILENO, &originalTermios);

    struct termios newt = originalTermios;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void restoreTerminalSettings() {
    const char restoreSeq[] = "\033[0m\033[?25h\033[?1000;1002;1006;1049l";
    write(STDOUT_FILENO, restoreSeq, sizeof(restoreSeq) - 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
}

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
