/**
 * @file obstacleSnake.c
 * @brief This source realizes the functions about obstacle snake.
 */

#include "GSnakeBInclude/Struct/GameAllRunningData.h"
#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/food.h"
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

/**
 * @brief Initialize the obstacle snake if it is enable.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsInit(GameAllRunningData *data) {
    for ( bool isRandWrongPos=true; isRandWrongPos; ) {
        srand((unsigned)time(NULL));
        data->obsSnkBody[0].x=rand()%(WIDE-3)+2;
        data->obsSnkBody[0].y=rand()%(HIGH-3)+2;
        isRandWrongPos=false;
        for ( int i=0; i<data->wallNum; i++ ) {
            if ( data->obsSnkBody[0].x==data->wall[i].x && \
                    data->obsSnkBody[0].y==data->wall[i].y ) {
                isRandWrongPos=true;
                break;
            }
        }
        if ( data->obsSnkBody[0].y<35 && data->obsSnkBody[0].y>25 && \
                data->obsSnkBody[0].x<13 && data->obsSnkBody[0].x>8 ) {
            isRandWrongPos=true;
        }
    }
    return;
}

/**
 * @brief Control the moving direction of obstacle snake.
 *
 * Control the moving direction of the obstacle snake
 * and move to the nearest food.
 *
 * @param[in,out] data All the game's data when the game is running.
 * @todo Unable to automatically avoid user snakes and
 *       walls, you need to add judgment code.
 */
void obsMoveDirecControl(GameAllRunningData *data) {
    int a=0,b=WIDE+HIGH+1;
    for ( int i=0; i<data->foodNum; i++ ) {
        a=abs(data->food[i].x-data->obsSnkBody[0].x)+\
          abs(data->food[i].y-data->obsSnkBody[0].y);
        if ( a<b ) {
            b=a;
            data->obsClosestFood=i;
        }
    }
    
    data->obsSnkNxtXDrc=data->obsSnkNxtYDrc=0;

    if ( data->obsSnkBody[0].x<data->food[data->obsClosestFood].x ) {
        data->obsSnkNxtXDrc=1;
    } else if ( data->obsSnkBody[0].x>data->food[data->obsClosestFood].x ) {
        data->obsSnkNxtXDrc=-1;
    } else if ( data->obsSnkBody[0].y<data->food[data->obsClosestFood].y ) {
        data->obsSnkNxtYDrc=1;
    } else if ( data->obsSnkBody[0].y>data->food[data->obsClosestFood].y ) {
        data->obsSnkNxtYDrc=-1;
    }
}

/**
 * @brief The position of the moving obstacle snake.
 *
 * The position of the obstacle's last body is copied from
 * the previous body, and the coordinates of the obstacle
 * head are added to GameAllRunningData.obsSnkNxtXDrc and
 * GameAllRunningData.obsSnkNxtYDrc.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsMove(GameAllRunningData *data) {
    for ( int i=data->obsSnkLeng-1; \
            i>0; i-- ) {
        data->obsSnkBody[i].x=data->obsSnkBody[i-1].x;
        data->obsSnkBody[i].y=data->obsSnkBody[i-1].y;
    }
    data->obsSnkBody[0].x+=data->obsSnkNxtXDrc;
    data->obsSnkBody[0].y+=data->obsSnkNxtYDrc;
}

/**
 * @brief Obstacle snake eat foods.
 * @param[in,out] data All the game's data when the game is running.
 */
void obsEatFood(GameAllRunningData *data) {
    int i;
    for ( i=0; i<data->foodNum; i++ ) {
        if ( data->obsSnkBody[0].x==data->food[i].x &&
                data->obsSnkBody[0].y==data->food[i].y ) {
            break;
        }
    }
    if ( i==data->foodNum ) {
        return;
    }
    foodInit(data,i);
    data->obsSnkLeng++;
}

/**
 * @brief Judge whether the obstacle snake eats the wall
 *        or the user snake.
 *
 * Judge whether the obstacle snake eats the wall or the
 * user snake. If obstacle snake eat them,it will change
 * the status(GameAllRunningData.obsState) of the obstacle
 * snake(to 1 or 2, see GameAllRunningData.obsState for
 * specific meaning) to describe the situation of obstacle.
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void obsEatWallsOrUserSnake(GameAllRunningData *data) {
    for ( int i=0; i<data->wallNum; i++ ) {
        if ( data->obsSnkBody[0].x==data->wall[i].x && \
                data->obsSnkBody[0].y==data->wall[i].y ) {
            data->obsState=1;
            return;
        }
    }

    for ( int i=0; i<data->usrSnkLeng; i++ ) {
        if ( data->obsSnkBody[0].x==data->usrSnkBody[i].x && \
                data->obsSnkBody[0].y==data->usrSnkBody[i].y ) {
            data->obsState=2;
            data->usrSnkIsEatingObsSnk=1;
            return;
        }
    }
}
