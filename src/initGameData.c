#include "include/global.h"
#include "include/Struct/GameAllRunningData.h"
#include "include/gameConfig.h"
#include "include/Functions/obstacleSnake.h"
#include "include/initGameData.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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
void initGameData(GameAllRunningData *data) {
    GameConfig config = {0};
    getGameConfig(&config);

    WIDE=config.scrnWide;
    HIGH=config.scrnHigh;

    data->usrSnkBody[0].x=WIDE/2;
    data->usrSnkBody[0].y=data->usrSnkBody[1].y=HIGH/2;
    data->usrSnkBody[1].x=WIDE/2-1;
    data->usrSnkLeng=2;
    
    data->obsSnkLeng=1;
    data->isEnableObs=config.isEnableObs;
    data->obsState=0;
    
    data->foodNum=config.foodNum;
    data->wallNum=config.wallNum;
    
    wallInit(data);
    
    if (data->isEnableObs) {
        obsInit(data);
    }
    
    for (uint64_t i = 0; i < data->foodNum; i++) {
        foodInit(data,i);
    }
    
    data->usrSnkNxtXDrc=1;
    
    data->isEnableEatSlfGmOver=config.isEnableEatSlfGmOver;

    data->speed=config.speed;

    data->obsIQ = config.obsIQ;
    data->obsSnkNxtXDrc=data->obsSnkNxtYDrc=0;
    data->obsClosestFood=0;

    data->histryHighestScr=config.histryHighestScr;

    data->usrSnkIsJumping=0;

    data->refreshTimes = 0;
}

/// Initialize all the unlimit-food-mode game's data
void initGameDataUnlimitFood(GameAllRunningData *data) {
    GameConfig config = {0};
    getGameConfig(&config);

    WIDE=config.scrnWide;
    HIGH=config.scrnHigh;

    data->usrSnkBody[0].x=WIDE/2;
    data->usrSnkBody[0].y=data->usrSnkBody[1].y=HIGH/2;
    data->usrSnkBody[1].x=WIDE/2-1;
    data->usrSnkLeng=2;
    
    data->obsSnkLeng=1;
    data->isEnableObs=false;
    data->obsState=0;
    
    data->foodNum=0;
    data->wallNum=0;
    
    data->usrSnkNxtXDrc=1;
    
    data->isEnableEatSlfGmOver=config.isEnableEatSlfGmOver;

    data->speed=config.speed;

    data->obsIQ = 0;
    data->obsSnkNxtXDrc=data->obsSnkNxtYDrc=0;
    data->obsClosestFood=0;

    data->histryHighestScr = UINTMAX_MAX;

    data->usrSnkIsJumping=0;

    data->refreshTimes = 0;
}
