/**
 * @file userSnake.c
 * @brief This source realizes the functions about snake's snake.
 */

#include "include/Struct/GameAllRunningData.h"
#include "include/terminal.h"
#include "include/painting.h"
#include "include/initGameData.h"
#include "include/global.h"
#include "include/Struct/Point.h"
#include "include/constants.h"
#include "include/button.h"
#include "include/userSnake.h"

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>

static void paintAll(int termW, int termH, void *data) {
    resetColor();
    fillBackground(termW, termH, NULL);
    wallPainting(data);
    gameAreaPainting(data);
}

/// Move user's snake, go through walls, come out on the other side
void userSnakeMoveCross(GameAllRunningData *data) {
    for (uint64_t i = data->usrSnkLeng - 1; i > 0; --i) {
        data->usrSnkBody[i]=data->usrSnkBody[i-1];
    }
    data->usrSnkBody[0].x+=data->usrSnkNxtXDrc;
    data->usrSnkBody[0].y+=data->usrSnkNxtYDrc;

    if (data->usrSnkBody[0].x == 1) {
        data->usrSnkBody[0].x = WIDE - 1;
    } else if (data->usrSnkBody[0].x == WIDE) {
        data->usrSnkBody[0].x = 2;
    } else if (data->usrSnkBody[0].y == 1) {
        data->usrSnkBody[0].y = HIGH - 1;
    } else if (data->usrSnkBody[0].y == HIGH) {
        data->usrSnkBody[0].y = 2;
    }
}

/// Move user's snake
void userSnakeMove(GameAllRunningData *data) {
    for (uint64_t i = data->usrSnkLeng - 1; i > 0; --i) {
        data->usrSnkBody[i]=data->usrSnkBody[i-1];
    }
    data->usrSnkBody[0].x+=data->usrSnkNxtXDrc;
    data->usrSnkBody[0].y+=data->usrSnkNxtYDrc;
}

static char parseMouse() {
    char selected = '\0';

    if (getchar() == '0') {
        uint64_t x, y;
        char option;
        scanf(";%lu;%lu%c", &x, &y, &option);

        if ((option == 'M' || option == 'm')
                && x >= WIDE + 1
                && x <= WIDE + 1 + 7) {

            Point termSize = terminalSize();

            if (termSize.y > 16) {
                if (RANGE_EQUAL(y, 5, 7)) {
                    selected = 'w';
                } else if (RANGE_EQUAL(y, 8, 10)) {
                    selected = 's';
                } else if (RANGE_EQUAL(y, 11, 13)) {
                    selected = 'a';
                } else if (RANGE_EQUAL(y, 14, 16)) {
                    selected = 'd';
                }
            }

            if (termSize.y > 21 && RANGE_EQUAL(y, 19, 21)) {
                selected = 'p';
            }

            if (termSize.y > 26 && RANGE_EQUAL(y, 23, 25)) {
                selected = 'o';
            }
        }
    }

    return selected;
}

/// Non-blocking keyboard check
static int keyboardHit() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
}

/**
 * @brief Control the direction of user's snake movement.
 *
 * | input | action |
 * | :---: | :----: |
 * | ←/A/7 | ← |
 * | ↑/W/5 | ↑ |
 * | ↓/S/8 | ↓ |
 * | →/D/9 | → |
 * | J | Jump |
 * | F/Tab | Fly |
 * | P | Pause |
 * | R | Repaint |
 * | Esc | Block game |
 * | Q/O | Game over |
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @return The state of the game when the game is running.
 * @retval 0 Game will be last running.
 * @retval 1 Game will be over.
 * @retval 1 Game will be over without pausing.
 */
int userSnakeMoveDirecControl(GameAllRunningData *data) {
    char key=0;

    if (atomic_exchange(&needRedraw, false)) {
        Point termSize = terminalSize();
        if (termSize.x < WIDE || termSize.y < HIGH) {
            if (screenTooSmallPainting(data)) {
                return -1;
            }
        }

        clearScreen();
        wallPainting(data);
        gameAreaPainting(data);
    }

    if ( data->usrSnkIsJumping ) {
        if ( data->usrSnkNxtXDrc<0 ) {
            data->usrSnkNxtXDrc++;
        } else if ( data->usrSnkNxtXDrc>0 ) {
            data->usrSnkNxtXDrc--;
        } else if ( data->usrSnkNxtYDrc<0 ) {
            data->usrSnkNxtYDrc++;
        } else if ( data->usrSnkNxtYDrc>0 ) {
            data->usrSnkNxtYDrc--;
        }
        data->usrSnkIsJumping=0;
    }

    while (keyboardHit() > 0) {
        key = getchar();
    }

    if (key == '\033' && getchar() == '[') {
        switch (getchar()) {
            case 'A':
                key = 'w';
                break;
            case 'B':
                key = 's';
                break;
            case 'C':
                key = 'd';
                break;
            case 'D':
                key = 'a';
                break;
            case '<':
                key = parseMouse();
                break;
        }
    }

    switch (key) {
    case 'a':
    case 'A':
    case '7':
        data->usrSnkNxtXDrc=-1;
        data->usrSnkNxtYDrc=0;
        break;

    case 'w':
    case 'W':
    case '5':
        data->usrSnkNxtXDrc=0;
        data->usrSnkNxtYDrc=-1;
        break;

    case 's':
    case 'S':
    case '8':
        data->usrSnkNxtXDrc=0;
        data->usrSnkNxtYDrc=1;
        break;

    case 'd':
    case 'D':
    case '9':
        data->usrSnkNxtXDrc=1;
        data->usrSnkNxtYDrc=0;
        break;

    case 'j':
    case 'J':
        if ( data->usrSnkNxtXDrc<0 ) {
            data->usrSnkNxtXDrc = -2;
        } else if ( data->usrSnkNxtXDrc>0 ) {
            data->usrSnkNxtXDrc = 2;
        } else if ( data->usrSnkNxtYDrc<0 ) {
            data->usrSnkNxtYDrc = -2;
        } else if ( data->usrSnkNxtYDrc>0 ) {
            data->usrSnkNxtYDrc = 2;
        }
        data->usrSnkIsJumping=1;
        break;

    case 'f':
    case 'F':
    case '\t':
        if ( data->usrSnkNxtXDrc<0 ) {
            data->usrSnkNxtXDrc = -2;
        } else if ( data->usrSnkNxtXDrc>0 ) {
            data->usrSnkNxtXDrc = 2;
        } else if ( data->usrSnkNxtYDrc<0 ) {
            data->usrSnkNxtYDrc = -2;
        } else if ( data->usrSnkNxtYDrc>0 ) {
            data->usrSnkNxtYDrc = 2;
        }
        data->usrSnkIsJumping=0;
        break;

    case 'p':
    case 'P':
        gamePausePainting(data, L"点击屏幕继续游戏");
        clearScreen();
        wallPainting(data);
        gameAreaPainting(data);
        break;

    case 'r':
    case 'R':
        clearScreen();
        wallPainting(data);
        gameAreaPainting(data);
        break;

    case 'o':
    case 'O':
    case 'q':
    case 'Q':
        {
            Point termSize = terminalSize();
            Button *b = buttonCreate();

            buttonTitle(b, "退出游戏");
            buttonHint(b, "您确定退出当前游戏吗？");
            buttonAdd(b, "确认退出");
            buttonAdd(b, "继续游戏");
            buttonBgDraw(b, paintAll, data);
            buttonInitial(b, 0);
            buttonUseAltBuffer(b, 0);

            ButtonResult res = buttonRun(b);
            if (res.confirmed && !res.selectedTop) {
                buttonFree(b);
                return 1;
            }
            buttonFree(b);

            clearScreen();
            resetColor();
            fillBackground(termSize.x, termSize.y, NULL);
            wallPainting(data);
            gameAreaPainting(data);

            printf("\033[?1002;1006h");
        }
        break;
    }

    return 0;
}

/// User's snake eat foods or death the obstacle snake's body
void userSnakeEatFood(GameAllRunningData *data) {
    for (uint64_t i=0; i<data->foodNum; i++ ) {
        if ( data->food[i].x==data->usrSnkBody[0].x
                && data->food[i].y==data->usrSnkBody[0].y ) {
            foodInit(data,i);
            data->usrSnkLeng++;
            data->usrSrc++;
        }
    }

    for (uint64_t i = 0; i < data->obsSnkLeng
            && data->obsState != 0; i++) {
        if ( data->obsSnkBody[i].x==data->usrSnkBody[0].x &&
                data->obsSnkBody[i].y==data->usrSnkBody[0].y ) {

            data->obsSnkBody[i].x =
                data->obsSnkBody[data->obsSnkLeng - 1].x;
            data->obsSnkBody[i].y =
                data->obsSnkBody[data->obsSnkLeng - 1].y;

            data->usrSnkLeng++;
            data->usrSrc++;
            data->obsSnkLeng--;
        }
    }
}

/// Determine if there's unlimited food at the coordinates
bool isUnlimitedFoodAt(uint64_t x, uint64_t y) {
    if (x < 2 || x > WIDE - 1 || y < 2 || y > HIGH - 1) {
        return false;
    }

    const uint64_t step = 2;
    const uint64_t left = 2, right = WIDE - 1;
    const uint64_t top = 2, bottom = HIGH - 1;

    uint64_t distLeft = x - left;
    uint64_t distRight = right - x;
    uint64_t distTop = y - top;
    uint64_t distBottom = bottom - y;

    uint64_t minDist = distLeft;
    if (distRight < minDist) {
        minDist = distRight;
    }
    if (distTop < minDist) {
        minDist = distTop;
    }
    if (distBottom < minDist) {
        minDist = distBottom;
    }

    uint64_t layer = minDist / step;

    uint64_t l = left + layer * step;
    uint64_t r = right - layer * step;
    uint64_t t = top + layer * step;
    uint64_t b = bottom - layer * step;

    if (l > r || t > b) {
        return false;
    }

    if (y == t && x >= l && x <= r) {
        return true;
    }
    if (t < b && y == b && x >= l && x <= r) {
        return true;
    }
    if (x == l && y >= t && y <= b) {
        return true;
    }
    if (l < r && x == r && y >= t && y <= b) {
        return true;
    }

    return false;
}

/// User's snake eat unlimited foods
void userSnakeEatUnlimitedFood(GameAllRunningData *data) {
    if (isUnlimitedFoodAt(
                data->usrSnkBody[0].x,
                data->usrSnkBody[0].y)) {
        data->usrSrc++;
        data->usrSnkLeng++;
    }
}

/**
 * @brief Is user's snake eating wall.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating wall.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatWall(const GameAllRunningData *data) {
    for (uint64_t i=0; i<data->wallNum; i++ ) {
        if ( data->usrSnkBody[0].x==data->wall[i].x &&
                data->usrSnkBody[0].y==data->wall[i].y) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Is user's snake eating obstacle snake body.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating obstacle snake body.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatObsSnake(const GameAllRunningData *data) {
    for (uint64_t i=1; i<data->obsSnkLeng; i++ ) {
        if ( data->usrSnkBody[0].x==data->obsSnkBody[i].x &&
                data->usrSnkBody[0].y==data->obsSnkBody[i].y) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Is user's snake eating its own body.
 *
 * @param[in] data All the game's data when the game is running.
 * @return bool Is user's snake eating its own body.
 * @retval true  Yes.
 * @retval false No.
 */
bool isUserSnakeEatSelf(const GameAllRunningData *data) {
    for (uint64_t i=1; i<data->usrSnkLeng; i++ ) {
        if ( data->usrSnkBody[0].x==data->usrSnkBody[i].x &&
                data->usrSnkBody[0].y==data->usrSnkBody[i].y ) {
            return true;
        }
    }
    return false;
}
