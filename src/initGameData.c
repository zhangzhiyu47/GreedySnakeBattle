#include "include/global.h"
#include "include/gameConfig.h"
#include "include/constants.h"
#include "include/obstacleSnake.h"
#include "include/painting.h"
#include "include/exitApp.h"
#include "include/terminal.h"
#include "include/initGameData.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ZERO(var) data->var = 0
#define INIT(var) data->var = config.var;

/// Update or initialize for the numberTH food
void foodInit(GameAllRunningData *data, int number) {
    for ( bool isRandWrongPos=true; isRandWrongPos; ) {
        data->food[number].x=rand()%(WIDE-3)+2;
        data->food[number].y=rand()%(HIGH-3)+2;
        isRandWrongPos=false;

        for (uint64_t i=0; i<data->usrSnkLeng; i++ ) {
            if ( data->usrSnkBody[i].x==data->food[number].x &&
                    data->usrSnkBody[i].y==data->food[number].y ) {
                isRandWrongPos=true;
                break;
            }
        }
        for (uint64_t i=0; i<data->foodNum; i++ ) {
            if ( data->food[i].x==data->food[number].x &&
                    data->food[i].y==data->food[number].y
                    && i != (uint64_t)number) {
                isRandWrongPos=true;
                break;
            }
        }
        for (uint64_t i=0; i<data->wallNum; i++ ) {
            if ( data->wall[i].x==data->food[number].x &&
                    data->wall[i].y==data->food[number].y ) {
                isRandWrongPos=true;
                break;
            }
        }
        for (uint64_t i=0; i<data->obsSnkLeng; i++ ) {
            if ( data->obsSnkBody[i].x==data->food[number].x &&
                    data->obsSnkBody[i].y==data->food[number].y ) {
                isRandWrongPos=true;
                break;
            }
        }
    }

    printf("\033[%lu;%luH" FOOD_C FOOD,
            data->food[number].y,
            data->food[number].x);
    resetColor();
}

/// Initialize all the obstacle walls
static void wallInit(GameAllRunningData *data) {
    for (uint64_t i = 0; i < data->wallNum; i++) {
        for (bool isRandWrongPos = true; isRandWrongPos;) {
            data->wall[i].x=rand()%(WIDE-3)+2;
            data->wall[i].y=rand()%(HIGH-3)+2;
            isRandWrongPos=false;

            for (uint64_t j=0; j<i; j++ ) {
                if ( data->wall[j].x==data->wall[i].x
                        && data->wall[j].y==data->wall[i].y ) {
                    isRandWrongPos=true;
                    break;
                }
            }

            if ( data->wall[i].y>27 && data->wall[i].y<32
                    && data->wall[i].x>7 && data->wall[i].x<13 ) {
                isRandWrongPos=true;
            }
        }
    }
}

/// Initialize all the classic-mode game's data
void initGameData(GameAllRunningData *data, GameMode mode) {
    GameConfig config = {0};
    getGameConfig(&config);

    Point termSize = terminalSize();

    WIDE = config.scrnWide;
    HIGH = config.scrnHigh;

    if (termSize.x < WIDE + ROCKER_BAR_WIDTH || termSize.y < HIGH) {
        if (termSize.x < MIN_TERMINAL_WIDE
                || termSize.y < MIN_TERMINAL_HIGH) {
            if (screenResizePainting()) {
                exitApp(EXIT_NORMAL, "", data);
            }
        }
        WIDE = termSize.x - ROCKER_BAR_WIDTH;
        HIGH = termSize.y;
    }

    data->usrSnkBody[0].x = WIDE / 2;
    data->usrSnkBody[0].y = data->usrSnkBody[1].y = HIGH / 2;
    data->usrSnkBody[1].x = WIDE / 2 - 1;
    data->usrSnkLeng = 2;

    data->usrSnkNxtXDrc = 1;
    data->usrSnkNxtYDrc = 0;

    INIT(speed);

    ZERO(refreshTimes);
    ZERO(usrSnkIsJumping);

    if (mode == MODE_CLASSIC) {
        data->obsSnkLeng = 1;
        INIT(isEnableObs);
        ZERO(obsState);

        INIT(foodNum);
        INIT(wallNum);

        wallInit(data);

        if (data->isEnableObs) {
            obsInit(data);
        }

        for (uint64_t i = 0; i < data->foodNum; i++) {
            foodInit(data,i);
        }

        INIT(isEnableEatSlfGmOver);

        INIT(obsIQ);
        ZERO(obsSnkNxtXDrc);
        ZERO(obsSnkNxtYDrc);
        ZERO(obsClosestFood);

        INIT(histryHighestScr);
    } else if (mode == MODE_UNLIMITED_FOOD) {
        ZERO(obsSnkLeng);
        ZERO(isEnableObs);
        ZERO(obsState);
        ZERO(isEnableEatSlfGmOver);
        ZERO(obsIQ);
        ZERO(obsSnkNxtXDrc);
        ZERO(obsSnkNxtYDrc);
        ZERO(obsClosestFood);

        data->histryHighestScr = UINTMAX_MAX;
    }
}
