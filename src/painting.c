#include "include/Struct/GameAllRunningData.h"
#include "include/GlobalVariable/globalVariable.h"
#include "include/Functions/terminal.h"
#include "include/Functions/painting.h"
#include "include/Functions/exitApp.h"
#include "include/Struct/Point.h"
#include "include/constants.h"
#include "include/gameConfig.h"
#include "include/logger.h"

#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/select.h>
#include <termios.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

#define MAX_DIGITS 128

#define WALL "▒"
#define WALL_COLOR RGB_BG(160, 82, 45) RGB_FG(107, 52, 16)

#define CURSOR_COLOR RGB_BG(74, 85, 104)

#define YOU_COLOR RGB_FG(183, 134, 11)

#define USER_SNAKE_HEAD "@"
#define USER_SNAKE_BODY "*"
#define USER_SNAKE_HEAD_C RGB_FG(1, 87, 155)
#define USER_SNAKE_BODY_C RGB_FG(3, 155, 229)

#define OBS_SNAKE_HEAD "$"
#define OBS_SNAKE_BODY "%%"
#define OBS_SNAKE_HEAD_C RGB_FG(0, 77, 64)
#define OBS_SNAKE_BODY_C RGB_FG(0, 137, 123)

#define FOOD "#"
#define FOOD_C RGB_FG(255, 111, 0)

#define ROCKER_OFFSET "\n\033[%luC", WIDE + 1
#define UP_C    RGB_BG(255, 0, 0)
#define DOWN_C  RGB_BG(0, 255, 0)
#define LEFT_C  RGB_BG(0, 128, 255)
#define RIGHT_C RGB_BG(255, 200, 0)
#define PAUSE_C RGB_BG(176, 190, 197)
#define QUIT_C  RGB_BG(158, 158, 158)
#define QUIT_WORD_C  RGB_FG(158, 158, 158)

/// Paint the buttons
static void buttonPainting() {
    Point termSize = terminalSize();
    resetColor();

    if (termSize.y > 16) {
        printf("\033[5;%luH", WIDE + 2);
        printf(UP_C"       " ROCKER_OFFSET);
        printf(UP_C"   ↑   " ROCKER_OFFSET);
        printf(UP_C"       " ROCKER_OFFSET);

        printf(DOWN_C"       " ROCKER_OFFSET);
        printf(DOWN_C"   ↓   " ROCKER_OFFSET);
        printf(DOWN_C"       " ROCKER_OFFSET);

        printf(LEFT_C"       " ROCKER_OFFSET);
        printf(LEFT_C"   ←   " ROCKER_OFFSET);
        printf(LEFT_C"       " ROCKER_OFFSET);

        printf(RIGHT_C"       " ROCKER_OFFSET);
        printf(RIGHT_C"   →   " ROCKER_OFFSET);
        printf(RIGHT_C"       " ROCKER_OFFSET);
    }

    if (termSize.y > 21) {
        printf("\033[19;%luH", WIDE + 2);
        printf(PAUSE_C"       " ROCKER_OFFSET);
        printf(PAUSE_C" ▶暂停 " ROCKER_OFFSET);
        printf(PAUSE_C"       " ROCKER_OFFSET);
    }

    if (termSize.y > 26) {
        printf("\033[23;%luH", WIDE + 2);
        printf(QUIT_C"       " ROCKER_OFFSET);
        printf(QUIT_C" ←退出 " ROCKER_OFFSET);
        printf(QUIT_C"       " ROCKER_OFFSET);
    }
}

/// Paint all the walls
void wallPainting(GameAllRunningData *data) {
    printf(WALL_COLOR "\033[1;1H");

    for (uint64_t i = 0; i < WIDE; i++) {
        printf(WALL);
    }
    printf("\n");

    for (uint64_t i = 0; i < HIGH - 2; ++i) {
        printf(WALL "\033[%luC" WALL "\n", WIDE - 2);
    }

    for (uint64_t i = 0; i < WIDE; i++) {
        printf(WALL);
    }

    for (uint64_t i = 0; i < data->wallNum; i++) {
        printf("\033[%lu;%luH" WALL, data->wall[i].y, data->wall[i].x);
    }

    buttonPainting();

    fflush(stdout);
    resetColor();
}

/**
 * @brief Paint the game interface when the game is running.
 *
 * Paint foods,user's snake,obstacle snake(if it is enable)
 * and user's score.
 *
 * @param[in] data All the game's data when the game is running.
 */
void gameInterfacePainting(GameAllRunningData const *const data) {
    for (uint64_t i = 0; i < data->foodNum; i++) {
        printf("\033[%lu;%luH",data->food[i].y, data->food[i].x);
        printf(FOOD_C FOOD);
    }

    for (int64_t i = data->obsSnkLeng - 1; i >= 0 &&
            data->isEnableObs == 1; --i) {
        printf("\033[%lu;%luH",data->obsSnkBody[i].y,
               data->obsSnkBody[i].x);
        if ( data->obsState!=0 ) {
            if ( data->obsState==2 ) {
                printf(FOOD_C FOOD);
            } else if (i) {
                printf(FOOD_C FOOD);
            } else {
                printf(WALL_COLOR WALL);
                resetColor();
            }
        } else if ( i==0 ) {
            printf(OBS_SNAKE_HEAD_C OBS_SNAKE_HEAD);
        } else {
            printf(OBS_SNAKE_BODY_C OBS_SNAKE_BODY);
        }
    }

    for (int64_t i = data->usrSnkLeng - 1; i >= 0; i--) {
        printf("\033[%lu;%luH",
                data->usrSnkBody[i].y, data->usrSnkBody[i].x);
        if ( i==0 ) {
            printf(USER_SNAKE_HEAD_C USER_SNAKE_HEAD);
        } else {
            printf(USER_SNAKE_BODY_C USER_SNAKE_BODY);
        }
    }
    resetColor();

    if (data->refreshTimes) {
        int playedTime = data->refreshTimes * data->speed / 1000 / 1000;
        printf("\033[1;%luH", WIDE + 1);
        printf(" [%02d:%02d]", playedTime / 60, playedTime % 60);
    }

    printf("\033[2;%luH", WIDE + 1);
    printf(" %04lu0分", data->usrSrc);

    if (data->usrSrc > data->histryHighestScr) {
        printf("\033[3;%luH 新记录!", WIDE + 1);
    }

    printf("\033[1;1H" WALL_COLOR WALL);
    resetColor();
    fflush(stdout);
}


/// Draw "↓You" to show which snake the user is controlling
void showWhichIsYoursSnake(GameAllRunningData *data) {
    Point start = {
        data->usrSnkBody[0].x,
        data->usrSnkBody[0].y - 1,
    };
    const int sleepTime = 250 * 1000;
    printf("\033[%lu;%luH", start.y, start.x);

    char you[] = "You";
    printf(YOU_COLOR"↓" CURSOR_COLOR" ");
    fflush(stdout);
    usleep(sleepTime * 2);

    for (size_t i = 0; i < strlen(you); ++i) {
        resetColor();
        printf("\b" YOU_COLOR"%c" CURSOR_COLOR" ", you[i]);
        fflush(stdout);
        usleep(sleepTime);
    }

    for (size_t i = 0; i < 4; ++i) {
        if (i % 2 == 0) {
            resetColor();
            printf("\b ");
        } else {
            printf("\b" CURSOR_COLOR" ");
        }
        fflush(stdout);

        usleep(sleepTime * 1.7);
    }

    for (uint64_t i = 0; i < strlen(you) + 1; ++i) {
        printf("\b\b" CURSOR_COLOR" ");
        resetColor();
        printf(" \b");
        fflush(stdout);
        usleep(sleepTime);
    }
    printf("\b ");

    int rawOffset = 0, colOffset = 0;

    rawOffset = (HIGH - 7) / 2;
    colOffset = (WIDE - 8) / 2;

    wallPainting(data);
    gameInterfacePainting(data);
    printf(RGB_FG(255, 179, 0)"\033[%d;1H", rawOffset);
    printf("\033[%dC ▄▄▄▄▄\n\r", colOffset);
    printf("\033[%dC█▀▀▀▀██▄\n\r", colOffset);
    printf("\033[%dC     ▄██\n\r", colOffset);
    printf("\033[%dC  █████\n\r", colOffset);
    printf("\033[%dC     ▀██\n\r", colOffset);
    printf("\033[%dC█▄▄▄▄██▀\n\r", colOffset);
    printf("\033[%dC ▀▀▀▀▀\n\r", colOffset);
    fflush(stdout);
    sleep(1);
    clearScreen();

    wallPainting(data);
    gameInterfacePainting(data);
    printf(RGB_FG(255, 111, 0)"\033[%d;1H", rawOffset);
    printf("\033[%dC ▄▄▄▄▄\n\r", colOffset);
    printf("\033[%dC█▀▀▀▀██▄\n\r", colOffset);
    printf("\033[%dC      ██\n\r", colOffset);
    printf("\033[%dC    ▄█▀\n\r", colOffset);
    printf("\033[%dC  ▄█▀\n\r", colOffset);
    printf("\033[%dC▄██▄▄▄▄▄\n\r", colOffset);
    printf("\033[%dC▀▀▀▀▀▀▀▀\n\r", colOffset);
    fflush(stdout);
    sleep(1);
    clearScreen();

    wallPainting(data);
    gameInterfacePainting(data);
    printf(RGB_FG(255, 23, 68)"\033[%d;1H", rawOffset);
    printf("\033[%dC  ▄▄▄\n\r", colOffset);
    printf("\033[%dC █▀██\n\r", colOffset);
    printf("\033[%dC   ██\n\r", colOffset);
    printf("\033[%dC   ██\n\r", colOffset);
    printf("\033[%dC   ██\n\r", colOffset);
    printf("\033[%dC▄▄▄██▄▄▄\n\r", colOffset);
    printf("\033[%dC▀▀▀▀▀▀▀▀\n\r", colOffset);
    fflush(stdout);
    sleep(1);
    clearScreen();

    rawOffset = (HIGH - 7) / 2;
    colOffset = (WIDE - 22) / 2;
    wallPainting(data);
    gameInterfacePainting(data);
    printf(RGB_FG(0, 230, 118)"\033[%d;1H", rawOffset);
    printf("\033[%dC   ▄▄▄▄    ▄▄▄▄     ▄▄\n\r", colOffset);
    printf("\033[%dC ██▀▀▀▀█  ██▀▀██    ██\n\r", colOffset);
    printf("\033[%dC██       ██    ██   ██\n\r", colOffset);
    printf("\033[%dC██  ▄▄▄▄ ██    ██   ██\n\r", colOffset);
    printf("\033[%dC██  ▀▀██ ██    ██   ▀▀\n\r", colOffset);
    printf("\033[%dC ██▄▄▄██  ██▄▄██    ▄▄\n\r", colOffset);
    printf("\033[%dC   ▀▀▀▀    ▀▀▀▀     ▀▀\n\r", colOffset);
    fflush(stdout);
    sleep(1);
    clearScreen();

    printf("\033[?1002;1006h");
    wallPainting(data);
    gameInterfacePainting(data);

    // Discard read buffer
    tcflush(STDIN_FILENO, TCIFLUSH);
}

/// Fill background
void fillBackground(int termW, int termH, void *context) {
    (void)context;

    setBackgroundColor();
    for (int i = 0; i < termW * termH / 4 + 1; ++i) {
        printf("    ");
    }
}

static volatile sig_atomic_t winCh = 0;

static void sigWinchHandler(int signum) {
    UNUSED(signum);
    winCh = 1;
}

static void screenTooSmallTip(GameConfig *config, Point termSize) {
    resetColor();
    clearScreen();

    printf("当前终端屏幕过小，游戏无法正常进行\n");

    printf("你的设置 %lux%lu, 当前 %lux%lu\n"
           "游戏需要至少 %lux%lu 的终端空间（含摇杆栏）\n",
           config->scrnWide, config->scrnHigh,
           termSize.x, termSize.y,
           config->scrnWide + ROCKER_BAR_WIDTH, config->scrnHigh);

    printf("您可以按 Q 退出应用。或按 q 退出游戏界面，"
           "进入 \"设置 -> 更多设置\" 调整游戏界面大小\n");
}

/// Ask user to zoom in screen or exit app
/// Return 0 if terminal size is sufficient, 1 to exit game interface
int screenTooSmallPainting(GameAllRunningData *data) {
    sigset_t set;
    struct sigaction sa, oldSa;
    char buf[4096] = {0};
    int retval = 0;

    GameConfig config = {0};
    getGameConfig(&config);

    sa.sa_handler = sigWinchHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGWINCH, &sa, &oldSa) == -1) {
        logger(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(1, "游戏出错", data);
    }

    sigfillset(&set);
    sigdelset(&set, SIGWINCH);
    sigdelset(&set, SIGINT);

    screenTooSmallTip(&config, terminalSize());

    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);

        int nReady = 
            pselect(STDIN_FILENO + 1, &rfds, NULL, NULL, NULL, &set);

        if (nReady < 0) {
            if (errno == EINTR) {
                if (winCh) {
                    winCh = 0;
                    Point termSize = terminalSize();
                    if (termSize.x < WIDE || termSize.y < HIGH) {
                        screenTooSmallTip(&config, termSize);
                    } else {
                        break;
                    }
                }
                continue;
            }
            logger(LOG_ERROR, "pselect: %s" HERE, strerror(errno));
            exitApp(1, "游戏出错", data);
        }

        if (nReady > 0 && FD_ISSET(STDIN_FILENO, &rfds)) {
            memset(buf, 0, 4096);

            int len = read(STDIN_FILENO, &buf, 4095);
            if (len == -1) {
                logger(LOG_ERROR, "read: %s" HERE, strerror(errno));
                exitApp(1, "游戏出错", data);
            } else if (len > 0) {
                if (strstr(buf, "Q")) {
                    exitApp(0, "", data);
                } else if (strstr(buf, "q")) {
                    retval = 1;
                    break;
                }
            }
        }
    }

    if (sigaction(SIGWINCH, &oldSa, NULL) == -1) {
        logger(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(1, "游戏出错", data);
    }

    return retval;
}

/// Draw the game pause screen
void gamePausePainting(GameAllRunningData *data) {
    resetColor();

    fillBackground(terminalSize().x, terminalSize().y, NULL);
    wallPainting(data);
    gameInterfacePainting(data);

    printf(QUIT_WORD_C"\033[%lu;%luH点击屏幕继续",
            (HIGH - 1) / 2, (WIDE - 12) / 2);
    printf("\033[?1002;1006h");
    fflush(stdout);

    usleep(40 * 1000);
    tcflush(STDIN_FILENO, TCIFLUSH);

    struct termios origTermios;

    tcgetattr(STDIN_FILENO, &origTermios);
    struct termios raw = origTermios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    char buf[32];
    while (1) {
        ssize_t len = read(STDIN_FILENO, buf, sizeof(buf));
        if (len > 0) {
            break;
        }
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);
    tcflush(STDIN_FILENO, TCIFLUSH);
}
