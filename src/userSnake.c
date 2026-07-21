/**
 * @file userSnake.c
 * @brief This source realizes the functions about snake's snake.
 */

#include "include/Struct/GameAllRunningData.h"
#include "include/Functions/standardIO.h"
#include "include/Functions/terminal.h"
#include "include/Functions/painting.h"
#include "include/Functions/food.h"
#include "include/GlobalVariable/globalVariable.h"
#include "include/Struct/Point.h"
#include "include/constants.h"
#include "include/button.h"

#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <poll.h>

static void paintAll(int termW, int termH, void *data) {
    resetColor();
    fillBackground(termW, termH, NULL);
    wallPainting(data);
    gameInterfacePainting(data);
}

/**
 * @brief Move user's snake.
 *
 * The position of the snake's last body is copied from
 * the previous body, and the coordinates of the snake
 * head are added to GameAllRunningData.usrSnkNxtXDrc and
 * GameAllRunningData.usrSnkNxtYDrc
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void userSnakeMove(GameAllRunningData *data) {
    for (uint64_t i=data->usrSnkLeng-1; i>0; i-- ) {
        data->usrSnkBody[i]=data->usrSnkBody[i-1];
    }
    data->usrSnkBody[0].x+=data->usrSnkNxtXDrc;
    data->usrSnkBody[0].y+=data->usrSnkNxtYDrc;
    return;
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

/**
 * @brief Control the direction of user's snake movement.
 *
 * | input | action |
 * | :---: | :----: |
 * | ←/A/7 | ← |
 * | ↑/W/5 | ↑ |
 * | ↓/S/8 | ↓ |
 * | →/D/9 | → |
 * | j/J | Jump |
 * | f/F/Tab | Fly |
 * | p/P | Pause |
 * | r/R | Repaint |
 * | Esc | Block game |
 * | o/O/q/Q | Game over |
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @return The state of the game when the game is running.
 * @retval 0 Game will be last running.
 * @retval 1 Game will be over.
 */
int userSnakeMoveDirecControl(GameAllRunningData *data) {
    char key=0;

    if (atomic_exchange(&needRedraw, false)) {
        Point termSize = terminalSize();
        if (termSize.x < WIDE || termSize.y < HIGH) {
            if (screenTooSmallPainting(data)) {
                return 1;
            }
        }

        clearScreen();
        wallPainting(data);
        gameInterfacePainting(data);
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

    while ( linuxKbhit() > 0 ) {
        key=getchar();
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
        gamePausePainting(data);
        clearScreen();
        wallPainting(data);
        gameInterfacePainting(data);
        break;

    case 'r':
    case 'R':
        clearScreen();
        wallPainting(data);
        gameInterfacePainting(data);
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
            gameInterfacePainting(data);

            printf("\033[?1002;1006h");
        }
        break;
    }

    return 0;
}

/**
 * @brief User's snake eat foods or death the obstacle
 *        snake's body(If it is enable).
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @todo When the user snake eats the body of the obstacle snake,
 *       this function simply reduces the length of the obstacle
 *       snake to reduce the length of the snake, but this method
 *       can not accurately show the snake that will be eaten when
 *       drawing the game interface. Need to modify part of the
 *       logic of the code and the logic of drawing the snake's body
 *       (@ref gameInterfacePainting function also needs to improve).
 */
void userSnakeEatFood(GameAllRunningData *data) {
    for (uint64_t i=0; i<data->foodNum; i++ ) {
        if ( data->food[i].x==data->usrSnkBody[0].x &&
                data->food[i].y==data->usrSnkBody[0].y ) {
            foodInit(data,i);
            data->usrSnkLeng++;
            data->usrSrc+=1;
        }
    }

    for (uint64_t i=0; i<data->obsSnkLeng &&
            data->obsState!=0; i++ ) {
        if ( data->obsSnkBody[i].x==data->usrSnkBody[0].x &&
                data->obsSnkBody[i].y==data->usrSnkBody[0].y ) {
            for (uint64_t j = i; j < data->obsSnkLeng; ++j) {
                data->obsSnkBody[j].x = data->obsSnkBody[j + 1].x;
                data->obsSnkBody[j].y = data->obsSnkBody[j + 1].y;
            }

            data->usrSnkLeng++;
            data->usrSrc += 1;
            data->obsSnkLeng--;
        }
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
