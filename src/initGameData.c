#include "include/GlobalVariable/globalVariable.h"
#include "include/Struct/GameAllRunningData.h"
#include "include/gameConfig.h"
#include "include/Functions/obstacleSnake.h"
#include "include/Functions/food.h"
#include "include/initGameData.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

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
/// TODO: Set the portal
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
