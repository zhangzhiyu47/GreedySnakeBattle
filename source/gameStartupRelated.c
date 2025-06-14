/**
 * @file gameStartupRelated.c
 * @brief This source realizes the functions about launching game.
 */

#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Struct/GameAllRunningData.h"
#include "GSnakeBInclude/Struct/GameConfig.h"
#include "GSnakeBInclude/Functions/obstacleSnake.h"
#include "GSnakeBInclude/Functions/standardIO.h"
#include "GSnakeBInclude/Functions/terminal.h"
#include "GSnakeBInclude/Functions/wall.h"
#include "GSnakeBInclude/Functions/food.h"
#include <stdio.h>

/**
 * @brief Initialize all the game's data(All data
 *        within GameAllRunningData).
 *
 * @param[in,out] data All the game's data when the game is running.
 * 
 * @todo
 *      | number | action |
 *      | :----: | :----: |
 *      | 1 | Set the portal |
 *      | 2 | Unlimited food |
 */
void initAllGameData(GameAllRunningData *data) {
    GameConfig config= {0};
    if ( isConfigFileOpenFail==false ) {
        FILE *fp=fopen(configFile,"r");
        if ( fp==NULL ) {
            printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");

            isConfigFileOpenFail=true;

            config=outlineModeConfig;

            blockWaitUserEnter();
            clearScreen();
        } else {
            fread(&config,sizeof(GameConfig),1,fp);
            fclose(fp);
            fp=NULL;
        }
    } else {
        config=outlineModeConfig;
    }

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
    
    if ( data->isEnableObs ) obsInit(data);
    
    for ( int i=0; i<data->foodNum; i++ ) foodInit(data,i);
    
    data->usrSnkNxtXDrc=1;
    data->usrSnkNxtYDrc=data->usrSrc=data->usrSnkGameEndState=0;
    
    data->isEnableEatSlfGmOver=config.isEnableEatSlfGmOver;
    data->speed=config.speed;
    data->minScrOpnVctryPnt=config.minScrOpnVctryPnt;
    data->obsSnkNxtXDrc=data->obsSnkNxtYDrc=0;
    data->histryHighestScr=config.histryHighestScr;
    data->usrSnkIsEatingObsSnk=0;
    data->obsClosestFood=0;
    data->usrSnkIsJumping=0;
}
