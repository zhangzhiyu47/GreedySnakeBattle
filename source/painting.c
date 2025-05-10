/**
 * @file painting.c
 * @brief This source realizes the functions about painting interface.
 */

#include "GSnakeBInclude/Struct/GameAllRunningData.h"
#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/standardIO.h"
#include "GSnakeBInclude/Functions/painting.h"
#include <stdio.h>

/**
 * @brief Paint all the walls.
 * 
 * Use @ref HIGH,@ref WIDE,GameAllRunningData.wallNum
 * and GameAllRunningData.wall to paint the outer fences
 * and obstacle walls.
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @attention The parameters are not checked, the wrong parameters
 *            will produce unpredictable results or crash.
 */
void wallPainting(GameAllRunningData *data) {
    printf("\033[%d;%dH",1,1);
    for ( int i=0; i<HIGH; i++ ) {
        for ( int j=0; j<WIDE; j++ ) {
            if ( i==HIGH-1 || i==0 || j==0 || j==WIDE-1 ) {
                printf("+");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    for ( int i=0; i<data->wallNum; i++ ) {
        printf("\033[%d;%dH+",data->wall[i].y,data->wall[i].x);
    }
    printf("\033[%d;%dH",HIGH,WIDE+1);
}

/**
 * @brief Paint the game interface when the game is running.
 *
 * Paint foods,user's snake,obstacle snake(if it is enable)
 * and user's score.
 *
 * @param[in] data All the game's data when the game is running.
 */
void gameInterfacePainting(GameAllRunningData const *const data) {
    for ( int i=0; i<data->foodNum; i++ ) {
        printf("\033[%d;%dH",data->food[i].y, data->food[i].x);
        printf("#");
    }
    for ( int i=data->obsSnkLeng-1; i>=0 && \
            data->isEnableObs==1; i-- ) {
        printf("\033[%d;%dH",data->obsSnkBody[i].y, \
               data->obsSnkBody[i].x);
        if ( data->obsState!=0 ) {
            if ( data->obsState==2 ) {
                printf("#");
            } else i?printf("#"):printf("+");
        } else if ( i==0 ) {
            printf("$");
        } else {
            printf("%%");
        }
    }
    for ( int i=data->usrSnkLeng-1; i>=0; i-- ) {
        printf("\033[%d;%dH",data->usrSnkBody[i].y, data->usrSnkBody[i].x);
        if ( i==0 ) {
            printf("@");
        } else {
            printf("*");
        }
    }

    printf("\033[%d;%dH",HIGH+1,1);
    printf("%05d分",data->usrSrc);
    if ( data->usrSrc/10>data->histryHighestScr )
        printf("\033[%d;%dH已超过历史记录!",HIGH+1,9);
    printf("\033[%d;%dH",HIGH, WIDE+1);
}

/**
 * @brief Paint the game-over interface and update the highest
 *        history score(If the configuration file open failed,
 *        it will start the Offline Mode).
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void gameOverPainting(GameAllRunningData *data) {
    printf("\033[%d;%dH",HIGH/2-1,(WIDE-12)/2);
    if ( data->usrSnkGameEndState==1 ) {
        printf("你赢了 太厉害了");
    } else {
        printf("\033[%d;%dH",HIGH/2-1,(WIDE-12)/2);
        printf("你输了 下次注意");
    }
    
    printf("\033[%d;%dH",HIGH+2,(WIDE-20)/2);
    printf("游戏结束 分数为%d\n",data->usrSrc);
    
    if ( data->usrSrc/10>data->histryHighestScr ) {
        printf("你真厉害，创造了新的记录，超过了历史最高记录%d分\n",data->histryHighestScr*10);
        if ( isConfigFileOpenFail==false ) {
            FILE *fp=fopen("./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data","r");
            GameConfig config={0};

            if ( fp==NULL ) {
                printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");
                isConfigFileOpenFail=true;
                outlineModeConfig.histryHighestScr=data->usrSrc/10;
                outlineModeConfig.wallNum=data->wallNum;
                outlineModeConfig.isEnableEatSlfGmOver=data->isEnableEatSlfGmOver;
                outlineModeConfig.isEnableObs=data->isEnableObs;
                outlineModeConfig.minScrOpnVctryPnt=data->minScrOpnVctryPnt;
                outlineModeConfig.foodNum=data->foodNum;
                outlineModeConfig.speed=data->speed;
                outlineModeConfig.scrnHigh=HIGH;
                outlineModeConfig.scrnWide=WIDE;
            } else {
                fread(&config,sizeof(GameConfig),1,fp);
                config.histryHighestScr=data->usrSrc/10;
                fclose(fp);
                fp=NULL;

                fp=fopen("./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data","w");

                if ( fp==NULL ) {
                    printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");
                    isConfigFileOpenFail=true;
                    outlineModeConfig=config;
                    outlineModeConfig.histryHighestScr=data->usrSrc/10;
                } else {
                    fwrite(&config,sizeof(GameConfig),1,fp);
                    fclose(fp);
                }
            }
        } else {
            outlineModeConfig.histryHighestScr=data->usrSrc/10;
        }
    }

    if ( data->usrSnkIsEatingObsSnk ) {
        printf("你真厉害，你吃到了系统小蛇\n");
    }

    blockWaitUserEnter();
}
