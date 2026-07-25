#include "include/Struct/GameAllRunningData.h"
#include "include/global.h"
#include "include/terminal.h"
#include "include/painting.h"
#include "include/exitApp.h"
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
#include <wchar.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

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

#define FOOD   "#"
#define FOOD_C RGB_FG(255, 111, 0)

#define ROCKER_OFFSET "\n\033[%luC", WIDE + 1
#define UP_C        RGB_BG(255, 0, 0)
#define DOWN_C      RGB_BG(0, 255, 0)
#define LEFT_C      RGB_BG(0, 128, 255)
#define RIGHT_C     RGB_BG(255, 200, 0)
#define PAUSE_C     RGB_BG(176, 190, 197)
#define QUIT_C      RGB_BG(158, 158, 158)
#define QUIT_WORD_C RGB_FG(158, 158, 158)

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

/// Paint unlimited food in concentric rectangular frames
static void unlimitedFoodPainting() {
    const uint64_t top = 2;
    const uint64_t left = 2;
    const uint64_t bottom = HIGH - 1;
    const uint64_t right = WIDE - 1;
    const uint64_t step = 2;

    printf(UNLIMIT_FOOD_C);

    for (uint64_t layer = 0;; ++layer) {
        uint64_t l = left + layer * step;
        uint64_t r = right - layer * step;
        uint64_t t = top + layer * step;
        uint64_t b = bottom - layer * step;

        if (l > r || t > b) {
            break;
        }

        printf("\033[%lu;%luH", t, l);
        for (uint64_t c = l; c <= r; ++c) {
            printf(UNLIMIT_FOOD);
        }

        for (uint64_t row = t + 1; row <= b; ++row) {
            printf("\033[%lu;%luH" UNLIMIT_FOOD, row, r);
        }

        if (t < b) {
            for (int64_t c = (int64_t)r; c >= (int64_t)l; --c) {
                printf("\033[%lu;%luH" UNLIMIT_FOOD, b, (uint64_t)c);
            }
        }

        if (l < r && t + 1 < b) {
            for (int64_t row = (int64_t)(b - 1); row > (int64_t)t; --row) {
                printf("\033[%lu;%luH" UNLIMIT_FOOD, (uint64_t)row, l);
            }
        }
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

    if (!data->foodNum) {
        resetColor();
        unlimitedFoodPainting();
    }

    resetColor();
    fflush(stdout);
}

/// Paint game area
void gameAreaPainting(GameAllRunningData const *const data) {
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
        uint64_t playedTime =
            data->refreshTimes * data->speed / 1000 / 1000;
        printf("\033[1;%luH", WIDE + 1);
        printf(" [%02lu:%02lu]", playedTime / 60, playedTime % 60);
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

    printf("您可以按 Q 退出应用\n");
    printf("或按 q 退出游戏界面，进入 \"设置 -> 更多设置\" 调整游戏界面大小\n");
    printf("手机端可以尝试两指放大屏幕，或关闭键盘\n");
}

/// Ask user to zoom in screen or exit app.
/// Return 0 if terminal size is sufficient, 1 to exit game interface
int screenTooSmallPainting(GameAllRunningData *data) {
    sigset_t set;
    struct sigaction sa, oldSa;
    char buf[64] = {0};
    int retval = 0;

    GameConfig config = {0};
    getGameConfig(&config);

    sa.sa_handler = sigWinchHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGWINCH, &sa, &oldSa) == -1) {
        logger(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错", data);
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
                    if (termSize.x < WIDE + ROCKER_BAR_WIDTH || termSize.y < HIGH) {
                        screenTooSmallTip(&config, termSize);
                    } else {
                        break;
                    }
                }
                continue;
            }
            logger(LOG_ERROR, "pselect: %s" HERE, strerror(errno));
            exitApp(EXIT_ERROR, "游戏出错", data);
        }

        if (nReady > 0 && FD_ISSET(STDIN_FILENO, &rfds)) {
            int len = read(STDIN_FILENO, &buf, 63);
            if (len == -1) {
                logger(LOG_ERROR, "read: %s" HERE, strerror(errno));
                exitApp(EXIT_ERROR, "游戏出错", data);
            } else if (len > 0) {
                if (strstr(buf, "Q")) {
                    exitApp(EXIT_NORMAL, "", data);
                } else if (strstr(buf, "q")) {
                    retval = 1;
                    break;
                }
            }
        }
    }

    if (sigaction(SIGWINCH, &oldSa, NULL) == -1) {
        logger(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错", data);
    }

    return retval;
}

static int checkTerminalSize(GameAllRunningData *data) {
    if (atomic_exchange(&needRedraw, false)) {
        Point termSize = terminalSize();
        if (termSize.x < WIDE || termSize.y < HIGH) {
            if (screenTooSmallPainting(data)) {
                return -1;
            }
        }
        return 1;
    }
    return 0;
}

/// Draw the game pause screen
void gamePausePainting(GameAllRunningData *data, const wchar_t *tip) {
    resetColor();
    fillBackground(terminalSize().x, terminalSize().y, NULL);
    wallPainting(data);
    gameAreaPainting(data);

    const size_t len = wcswidth(tip, -1);
    printf(QUIT_WORD_C"\033[%lu;%luH%ls",
            (HIGH + 1 - 1) / 2, (WIDE + 1 - len) / 2, tip);
    printf("\033[?1002;1006h");
    fflush(stdout);

    usleep(40 * 1000);
    tcflush(STDIN_FILENO, TCIFLUSH);

    char buf[32];
    while (1) {
        ssize_t len = read(STDIN_FILENO, buf, sizeof(buf));
        if (len > 0) {
            break;
        }
    }

    usleep(40 * 1000);
    tcflush(STDIN_FILENO, TCIFLUSH);
}

static void screenResizeTip(Point termSize) {
    resetColor();
    clearScreen();

    printf("当前终端屏幕过小，游戏无法正常进行\n");

    printf("最低 %lux%lu, 当前 %lux%lu\n",
           MIN_TERMINAL_WIDE, MIN_TERMINAL_HIGH,
           termSize.x, termSize.y);

    printf("您可以按 Q 退出应用\n");
    printf("手机端可以尝试两指放大屏幕，或关闭键盘\n");
}

/// Block until terminal meets minimum size or user presses Q to quit.
/// Return 0 for the size of screen is OK now, 1 for quit app
int screenResizePainting() {
    sigset_t set;
    struct sigaction sa, oldSa;
    char buf[64] = {0};
    int retval = 0;

    sa.sa_handler = sigWinchHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGWINCH, &sa, &oldSa) == -1) {
        logger(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错", NULL);
    }

    sigfillset(&set);
    sigdelset(&set, SIGWINCH);
    sigdelset(&set, SIGINT);

    screenResizeTip(terminalSize());

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

                    if (termSize.x < MIN_TERMINAL_WIDE
                            || termSize.y < MIN_TERMINAL_HIGH) {
                        screenResizeTip(termSize);
                    } else {
                        break;
                    }
                }
                continue;
            }
            logger(LOG_ERROR, "pselect: %s" HERE, strerror(errno));
            exitApp(EXIT_ERROR, "游戏出错", NULL);
        }

        if (nReady > 0 && FD_ISSET(STDIN_FILENO, &rfds)) {
            int len = read(STDIN_FILENO, &buf, 63);
            if (len == -1) {
                logger(LOG_ERROR, "read: %s" HERE, strerror(errno));
                exitApp(EXIT_ERROR, "游戏出错", NULL);
            } else if (len > 0) {
                if (strstr(buf, "Q") || strstr(buf, "q")) {
                    retval = 1;
                    break;
                }
            }
        }
    }

    if (sigaction(SIGWINCH, &oldSa, NULL) == -1) {
        logger(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错", NULL);
    }

    return retval;
}

static void threePainting(int rawOffset, int colOffset) {
    printf(RGB_FG(255, 179, 0)"\033[%d;1H", rawOffset);
    printf("\033[%dC\033[1C▄▄▄▄▄\n\r", colOffset);
    printf("\033[%dC█▀▀▀▀██▄\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C\033[1C\033[1C▄██\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C█████\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C\033[1C\033[1C▀██\n\r", colOffset);
    printf("\033[%dC█▄▄▄▄██▀\n\r", colOffset);
    printf("\033[%dC\033[1C▀▀▀▀▀\n\r", colOffset);
}

static void twoPainting(int rawOffset, int colOffset) {
    printf(RGB_FG(255, 111, 0)"\033[%d;1H", rawOffset);
    printf("\033[%dC\033[1C▄▄▄▄▄\n\r", colOffset);
    printf("\033[%dC█▀▀▀▀██▄\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C\033[1C\033[1C\033[1C██\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C\033[1C▄█▀\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C▄█▀\n\r", colOffset);
    printf("\033[%dC▄██▄▄▄▄▄\n\r", colOffset);
    printf("\033[%dC▀▀▀▀▀▀▀▀\n\r", colOffset);
}

static void onePainting(int rawOffset, int colOffset) {
    printf(RGB_FG(255, 23, 68)"\033[%d;1H", rawOffset);
    printf("\033[%dC\033[1C\033[1C▄▄▄\n\r", colOffset);
    printf("\033[%dC\033[1C█▀██\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C██\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C██\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C██\n\r", colOffset);
    printf("\033[%dC▄▄▄██▄▄▄\n\r", colOffset);
    printf("\033[%dC▀▀▀▀▀▀▀▀\n\r", colOffset);
}

static void goPainting(int rawOffset, int colOffset) {
    printf(RGB_FG(0, 230, 118)"\033[%d;1H", rawOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C▄▄▄▄\033[1C\033[1C\033[1C\033[1C▄▄▄▄\033[1C\033[1C\033[1C\033[1C\033[1C▄▄\n\r", colOffset);
    printf("\033[%dC\033[1C██▀▀▀▀█\033[1C\033[1C██▀▀██\033[1C\033[1C\033[1C\033[1C██\n\r", colOffset);
    printf("\033[%dC██\033[1C\033[1C\033[1C\033[1C\033[1C\033[1C\033[1C██\033[1C\033[1C\033[1C\033[1C██\033[1C\033[1C\033[1C██\n\r", colOffset);
    printf("\033[%dC██\033[1C\033[1C▄▄▄▄\033[1C██\033[1C\033[1C\033[1C\033[1C██\033[1C\033[1C\033[1C██\n\r", colOffset);
    printf("\033[%dC██\033[1C\033[1C▀▀██\033[1C██\033[1C\033[1C\033[1C\033[1C██\033[1C\033[1C\033[1C▀▀\n\r", colOffset);
    printf("\033[%dC\033[1C██▄▄▄██\033[1C\033[1C██▄▄██\033[1C\033[1C\033[1C\033[1C▄▄\n\r", colOffset);
    printf("\033[%dC\033[1C\033[1C\033[1C▀▀▀▀\033[1C\033[1C\033[1C\033[1C▀▀▀▀\033[1C\033[1C\033[1C\033[1C\033[1C▀▀\n\r", colOffset);
}

static int numberPainting(
        GameAllRunningData *data, int rawOffset, int colOffset,
        void (*numberPainter)(int, int)) {
    const int refreshTimes = 10;

    clearScreen();

    wallPainting(data);
    gameAreaPainting(data);
    numberPainter(rawOffset, colOffset);

    fflush(stdout);

    for (int i = 0; i < refreshTimes; ++i) {
        int retval = checkTerminalSize(data);
        if (retval == 1) {
            clearScreen();

            wallPainting(data);
            gameAreaPainting(data);
            numberPainter(rawOffset, colOffset);

            fflush(stdout);
        } else if (retval == -1) {
            return 1;
        }
        usleep(1000 * 1000 / refreshTimes);
    }

    return 0;
}

static void drawYou(const char *you, int which, int cursor) {
    printf(YOU_COLOR"↓");

    for (int i = 0; i < which - 1; ++i) {
        putchar(you[i]);
    }

    if (cursor) {
        printf(CURSOR_COLOR" ");
    } else {
        printf(" ");
    }
    resetColor();
}

/// Draw "↓You" to show which snake the user is controlling.
/// Return 0 for continue game, 1 for exit game interface
int showWhichIsYoursSnake(GameAllRunningData *data) {
    clearScreen();
    wallPainting(data);
    gameAreaPainting(data);

    const Point start = {
        data->usrSnkBody[0].x,
        data->usrSnkBody[0].y - 1,
    };
    const int sleepTime = 250 * 1000;
    const char *you = "You";
    const int refreshTimes = 10;

    for (size_t i = 0; i < strlen(you) + 2; ++i) {
        printf("\033[%lu;%luH", start.y, start.x);
        drawYou(you, i, 1);
        fflush(stdout);

        for (int j = 0; j < refreshTimes; ++j) {
            int retval = checkTerminalSize(data);
            if (retval == 1) {
                clearScreen();

                wallPainting(data);
                gameAreaPainting(data);

                printf("\033[%lu;%luH", start.y, start.x);
                drawYou(you, i, 1);
                fflush(stdout);
            } else if (retval == -1) {
                return 1;
            }
            usleep(sleepTime / refreshTimes);
        }
    }

    for (int i = 0; i < 4; ++i) {
        printf("\033[%lu;%luH", start.y, start.x);
        if (i % 2 == 0) {
            resetColor();
            drawYou(you, strlen(you) + 2, 0);
        } else {
            drawYou(you, strlen(you) + 2, 1);
        }
        fflush(stdout);

        for (int j = 0; j < refreshTimes; ++j) {
            int retval = checkTerminalSize(data);
            if (retval == 1) {
                clearScreen();

                wallPainting(data);
                gameAreaPainting(data);

                printf("\033[%lu;%luH", start.y, start.x);
                if (i % 2 == 0) {
                    resetColor();
                    drawYou(you, strlen(you) + 2, 0);
                } else {
                    drawYou(you, strlen(you) + 2, 1);
                }
                fflush(stdout);
            } else if (retval == -1) {
                return 1;
            }
            usleep(sleepTime * 1.7 / refreshTimes);
        }
    }

    for (size_t i = strlen(you) + 1; i > 0; --i) {
        printf("\033[%lu;%luH", start.y, start.x);
        drawYou(you, i, 1);
        resetColor();
        putchar(' ');
        fflush(stdout);

        for (int j = 0; j < refreshTimes; ++j) {
            int retval = checkTerminalSize(data);
            if (retval == 1) {
                clearScreen();

                wallPainting(data);
                gameAreaPainting(data);

                printf("\033[%lu;%luH", start.y, start.x);
                drawYou(you, i, 1);
                resetColor();
                putchar(' ');
                fflush(stdout);
            } else if (retval == -1) {
                return 1;
            }
            usleep(sleepTime / refreshTimes);
        }
    }
    printf(CURSOR_COLOR"\033[%lu;%luH ", start.y, start.x);
    resetColor();
    putchar(' ');
    fflush(stdout);
    usleep(sleepTime);

    int rawOffset = 0, colOffset = 0;
    rawOffset = (HIGH + 1 - 7) / 2;
    colOffset = (WIDE + 1 - 8) / 2;
    if (numberPainting(data, rawOffset, colOffset, threePainting)) {
        return 1;
    };

    if (numberPainting(data, rawOffset, colOffset, twoPainting)) {
        return 1;
    };

    if (numberPainting(data, rawOffset, colOffset, onePainting)) {
        return 1;
    };

    rawOffset = (HIGH + 1 - 7) / 2;
    colOffset = (WIDE + 1 - 22) / 2;
    if (colOffset <= 0) {
        colOffset = (MIN_TERMINAL_WIDE + 1 - 22) / 2;
    }
    if (numberPainting(data, rawOffset, colOffset, goPainting)) {
        return 1;
    };

    printf("\033[?1002;1006h");
    clearScreen();
    wallPainting(data);
    gameAreaPainting(data);

    // Discard read buffer
    tcflush(STDIN_FILENO, TCIFLUSH);

    return 0;
}
