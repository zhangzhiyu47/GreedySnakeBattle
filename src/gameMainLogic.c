#include "include/global.h"
#include "include/obstacleSnake.h"
#include "include/userSnake.h"
#include "include/painting.h"
#include "include/terminal.h"
#include "include/constants.h"
#include "include/gameMainLogic.h"

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

/// The main logic of the classic-mode game
void gameMainLogic(GameAllRunningData *data) {
    Point termSize = terminalSize();
    if (termSize.x < WIDE + ROCKER_BAR_WIDTH || termSize.y < HIGH) {
        if (screenTooSmallPainting(data)) {
            return;
        }
    }

    allPainting(data);
    if (showWhichIsYoursSnake(data)) {
        return;
    }
    usleep(data->speed);

    while ( data->usrSnkBody[0].x<WIDE && data->usrSnkBody[0].x>1 &&
            data->usrSnkBody[0].y<HIGH && data->usrSnkBody[0].y>1 ) {

        int retval = userSnakeMoveDirecControl(data);
        if (retval == 1) {
            break;
        } else if (retval == -1) {
            return;
        }

        printf("\033[%lu;%luH ",
                data->usrSnkBody[data->usrSnkLeng-1].y,
                data->usrSnkBody[data->usrSnkLeng-1].x);
        userSnakeMove(data);

        if (data->isEnableObs && !data->obsState) {
            printf("\033[%lu;%luH ",
                    data->obsSnkBody[data->obsSnkLeng-1].y,
                    data->obsSnkBody[data->obsSnkLeng-1].x);
            obsMoveDirecControl(data);
            obsMove(data);
        }

        /*
        printf("\033[%lu;%luH@",
                data->usrSnkBody[0].y,
                data->usrSnkBody[0].x);
        printf("\033[%lu;%luH*",
                data->usrSnkBody[1].y,
                data->usrSnkBody[1].x);
        */
        gameAreaPainting(data);

        if (!data->obsState) {
            obsEatFood(data);
            obsEatWallsOrUserSnake(data);
        }

        if (isUserSnakeEatSelf(data)
                && data->isEnableEatSlfGmOver!=0 ) {
            break;
        }

        if ( data->usrSrc >= HIGH * WIDE
                - (HIGH + WIDE) * 2
                - data->foodNum
                - data->wallNum
                - data->isEnableObs * data->obsSnkLeng) {
            break;
        }

        if (data->isEnableObs
                && !data->obsState
                && data->obsSnkLeng >= HIGH * WIDE
                    - (HIGH + WIDE) * 2
                    - data->foodNum
                    - data->wallNum
                    - data->usrSnkLeng
                    - data->obsSnkLeng) {
            break;
        }

        if (data->usrSnkLeng >= SNAKE_MAX_LENGTH) {
            break;
        }

        if (data->obsSnkLeng >= SNAKE_MAX_LENGTH) {
            break;
        }

        if ((isUserSnakeEatWall(data) && data->wallNum)
                || 
                (isUserSnakeEatObsSnake(data)
                 && data->isEnableObs
                 && !data->obsState) ) {
            break;
        }

        userSnakeEatFood(data);

        fflush(stdout);
        usleep(data->speed);
        data->refreshTimes++;
    }

    if (data->usrSrc > data->histryHighestScr) {
        GameConfig config = {0};

        getGameConfig(&config);
        config.histryHighestScr = data->usrSrc;
        setGameConfig(&config);
    }

    gamePausePainting(data, L"游戏结束，点击屏幕继续");
}

/// The main logic of the unlimited-mode game
void gameMainLogicUnlimitedMode(GameAllRunningData *data) {
    Point termSize = terminalSize();
    if (termSize.x < WIDE + ROCKER_BAR_WIDTH || termSize.y < HIGH) {
        if (screenTooSmallPainting(data)) {
            return;
        }
    }

    allPainting(data);
    if (showWhichIsYoursSnake(data)) {
        return;
    }
    usleep(data->speed);

    while ( data->usrSnkBody[0].x<WIDE && data->usrSnkBody[0].x>1 &&
            data->usrSnkBody[0].y<HIGH && data->usrSnkBody[0].y>1 ) {

        int retval = userSnakeMoveDirecControl(data);
        if (retval == 1) {
            break;
        } else if (retval == -1) {
            return;
        }

        if (isUnlimitedFoodAt(
                    data->usrSnkBody[data->usrSnkLeng - 1].x,
                    data->usrSnkBody[data->usrSnkLeng - 1].y)) {
            printf("\033[%lu;%luH" UNLIMIT_FOOD_C UNLIMIT_FOOD,
                    data->usrSnkBody[data->usrSnkLeng - 1].y,
                    data->usrSnkBody[data->usrSnkLeng - 1].x);
            resetColor();
        } else {
            printf("\033[%lu;%luH ",
                    data->usrSnkBody[data->usrSnkLeng - 1].y,
                    data->usrSnkBody[data->usrSnkLeng - 1].x);
        }
        userSnakeMoveCross(data);

        gameAreaPainting(data);

        if (data->usrSrc >= HIGH * WIDE - (HIGH + WIDE) * 2) {
            break;
        }

        if (data->usrSnkLeng >= SNAKE_MAX_LENGTH) {
            break;
        }

        userSnakeEatUnlimitedFood(data);

        fflush(stdout);
        usleep(data->speed);
        data->refreshTimes++;
    }

    gamePausePainting(data, L"游戏结束，点击屏幕继续");
}
