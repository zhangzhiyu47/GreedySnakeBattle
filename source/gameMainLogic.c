/**
 * @file gameMainLogic.c
 * @brief This source realizes the functions about game main logic.
 */

#include "Include/Compat/snakeFullCompat.h"

#include "Include/GlobalVariable/globalVariable.h"
#include "Include/Struct/GameAllRunningData.h"
#include "Include/Functions/obstacleSnake.h"
#include "Include/Functions/userSnake.h"
#include "Include/Functions/painting.h"
#include <stdio.h>

/**
 * @brief The main loop of the game process.
 *
 * The main loop of the game process. Move the user
 * and obstacle snake (if he is enabled) (hereinafter
 * referred to as snake for short), execute the
 * printing game interface, the snake eats food,
 * eats the wall, whether it eats itself, judges
 * whether it is dead, quits the game and other game
 * behaviors.
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @bug When the game is running, when the
 *      snake's body is large enough, the
 *      game will get stuck there unless the
 *      signal is sent to stop.
 */
void mainLoopOfGameProcess(GameAllRunningData *data) {
    while ( data->usrSnkBody[0].x<WIDE && data->usrSnkBody[0].x>1 && \
            data->usrSnkBody[0].y<HIGH && data->usrSnkBody[0].y>1 ) {
        
        if ( userSnakeMoveDirecControl(data) ) {
            break;
        }
        
        printf("\033[%d;%dH",data->usrSnkBody[data->usrSnkLeng-1].y,data->usrSnkBody[data->usrSnkLeng-1].x);
        printf(" ");
        userSnakeMove(data);
        
        if ( data->isEnableObs && !data->obsState ) {
            printf("\033[%d;%dH",data->obsSnkBody[data->obsSnkLeng-1].y, \
                   data->obsSnkBody[data->obsSnkLeng-1].x);
            printf(" ");
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
        
        if ( data->usrSrc/10==data->minScrOpnVctryPnt ) {
            printf("\033[%d;%dH",HIGH,(WIDE-1)/2+1);
            printf("$");
            printf("\033[%d;%dH",HIGH,WIDE+1);
        }
        
        if ( data->usrSrc/10>=data->minScrOpnVctryPnt \
                && data->usrSnkBody[0].x==(WIDE-1)/2+1 && \
                data->usrSnkBody[0].y==HIGH ) {
            data->usrSnkGameEndState=1;
            break;
        }
        if ( data->usrSrc/10>=HIGH*WIDE-(HIGH+WIDE)*2-40- \
                data->isEnableObs*data->obsSnkLeng ) {
            data->usrSnkGameEndState=1;
            break;
        }
        if ( data->isEnableObs && !data->obsState \
                && data->obsSnkLeng>=HIGH*WIDE-(HIGH+WIDE)*2-40- \
                data->isEnableObs*data->obsSnkLeng ) {
            data->usrSnkGameEndState=0;
            break;
        }
        if ( (isUserSnakeEatWall(data) && data->wallNum) \
                || (isUserSnakeEatObsSnake(data) && \
                data->isEnableObs && !data->obsState) ) {
            break;
        }
        
        userSnakeEatFood(data);

        fflush(stdout);
        usleep(data->speed);
    }

    gameOverPainting(data);
}
