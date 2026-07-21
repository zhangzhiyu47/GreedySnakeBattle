#include "include/GlobalVariable/globalVariable.h"
#include "include/Struct/GameAllRunningData.h"
#include "include/Functions/obstacleSnake.h"
#include "include/Functions/userSnake.h"
#include "include/Functions/painting.h"
#include "include/Functions/terminal.h"
#include "include/Struct/Point.h"
#include "include/constants.h"
#include "include/gameMainLogic.h"

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

/**
 * @brief The main logic of the game
 *
 * @bug If snake's body is large enough, the
 *      game will get stuck there unless the
 *      signal is sent to stop.
 */
void gameMainLogic(GameAllRunningData *data) {
    Point termSize = terminalSize();
    if (termSize.x < WIDE + ROCKER_BAR_WIDTH || termSize.y < HIGH) {
        if (screenTooSmallPainting(data)) {
            return;
        }
    }

    showWhichIsYoursSnake(data);
    usleep(data->speed * 2);

    while ( data->usrSnkBody[0].x<WIDE && data->usrSnkBody[0].x>1 &&
            data->usrSnkBody[0].y<HIGH && data->usrSnkBody[0].y>1 ) {

        if ( userSnakeMoveDirecControl(data) ) {
            break;
        }

        printf("\033[%lu;%luH ",
                data->usrSnkBody[data->usrSnkLeng-1].y,
                data->usrSnkBody[data->usrSnkLeng-1].x);
        userSnakeMove(data);

        if ( data->isEnableObs && !data->obsState ) {
            printf("\033[%lu;%luH ",
                    data->obsSnkBody[data->obsSnkLeng-1].y,
                    data->obsSnkBody[data->obsSnkLeng-1].x);
            obsMoveDirecControl(data);
            obsMove(data);
        }

        gameInterfacePainting(data);
        if ( !data->obsState ) {
            obsEatFood(data);
            obsEatWallsOrUserSnake(data);
        }

        if ( isUserSnakeEatSelf(data) && data->isEnableEatSlfGmOver!=0 ) {
            break;
        }

        if ( data->usrSrc >= HIGH * WIDE - (HIGH + WIDE) * 2 - 40
                - data->isEnableObs*data->obsSnkLeng ) {
            data->usrSnkGameEndState=1;
            break;
        }

        if ( data->isEnableObs && !data->obsState
                && data->obsSnkLeng>=HIGH*WIDE-(HIGH+WIDE)*2-40-
                data->isEnableObs*data->obsSnkLeng ) {
            data->usrSnkGameEndState=0;
            break;
        }

        if ( (isUserSnakeEatWall(data) && data->wallNum)
                || (isUserSnakeEatObsSnake(data) &&
                data->isEnableObs && !data->obsState) ) {
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

    gamePausePainting(data);
}
