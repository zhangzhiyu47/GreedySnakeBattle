/**
 * @file gameSetConfiguration.c
 * @brief This source realizes the functions
 *        about game settings and configurations.
 */

#include "GSnakeBInclude/Struct/GameConfig.h"
#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/terminal.h"
#include <stdio.h>

/**
 * @brief Set GameConfig.isEnableEatSlfGmOver.
 * 
 * | number | setting |
 * | :----: | :-----: |
 * | 1 | Enable |
 * | others | disable |
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_isEnableEatSlfGmOver(GameConfig *config) {
    printf("请输入0->否,非0任何数->是：");
    fflush(stdout);
    int isEnable=1;

    scanf("%d",&isEnable);
    while ( getchar()!='\n' );

    config->isEnableEatSlfGmOver=isEnable?1:0;
}

/**
 * @brief Set GameConfig.speed.
 *
 * Read from the user in seconds and store in microseconds.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_speed(GameConfig *config) {
    printf("请输入0.2-2间的小数（精确到两位小数）：");
    fflush(stdout);
    double sspeed=0.0;

    scanf("%lf",&sspeed);
    while ( getchar()!='\n' );

    muint_t uspeed;
    if ( sspeed>=0.2 && sspeed<=2.0 ) {
        uspeed=sspeed*1000u*1000u;
        uspeed-=uspeed%(10*1000);
        config->speed=uspeed;
    }
}

/**
 * @brief Set GameConfig.minScrOpnVctryPnt.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_minScrOpnVctryPnt(GameConfig *config) {
    printf("请输入30-500分的整数：");
    fflush(stdout);
    int minScrOpnVctryPnt=0;

    scanf("%d",&minScrOpnVctryPnt);
    while ( getchar()!='\n' );

    if ( minScrOpnVctryPnt>=30 && minScrOpnVctryPnt<=500 ) {
        minScrOpnVctryPnt/=10;
        config->minScrOpnVctryPnt=minScrOpnVctryPnt;
    }
}

/**
 * @brief Set GameConfig.foodNum.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_foodNum(GameConfig *config) {
    printf("请输入1-20的整数：");
    fflush(stdout);
    int foodNum=0;

    scanf("%d",&foodNum);
    while ( getchar()!='\n' );

    if ( foodNum>=1 && foodNum<=20 ) {
        config->foodNum=foodNum;
    }
}

/**
 * @brief Set GameConfig.wallNum.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_wallNum(GameConfig *config) {
    printf("请输入0-15的整数：");
    fflush(stdout);
    int wallNum=0;

    scanf("%d",&wallNum);
    while ( getchar()!='\n' );

    if ( wallNum>=0 && wallNum<=15 ) {
        config->wallNum=wallNum;
    }
}

/**
 * @brief Set GameConfig.isEnableObs.
 * 
 * | number | setting |
 * | :----: | :-----: |
 * | 1 | Enable |
 * | others | disable |
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_isEnableObs(GameConfig *config) {
    printf("请输入0->否,非0任何数->是：");
    fflush(stdout);
    int isEnableObs=1;

    scanf("%d",&isEnableObs);
    while ( getchar()!='\n' );

    config->isEnableObs=isEnableObs?1:0;
}

/**
 * @brief Set GameConfig.scrnHigh and GameConfig.scrnWide.
 *
 * Set GameConfig.scrnHigh and GameConfig.scrnWide.By the
 * way,set global variables @ref HIGH and @ref WIDE.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_scrnHigh_scrnWide(GameConfig *config) {
    printf("请输入游戏界面的垂直长度(5~40)：");
    fflush(stdout);
    int high=0;

    scanf("%d",&high);
    while ( getchar()!='\n' );

    if ( high<=40 && high>=5 ) {
        config->scrnHigh=HIGH=high;
    }
    
    printf("请输入游戏界面的水平长度(10~80)：");
    fflush(stdout);
    int wide=0;

    scanf("%d",&wide);
    while ( getchar()!='\n' );

    if ( wide<=80 && wide>=10 ) {
        config->scrnWide=WIDE=wide;
    }
}

/**
 * @brief Set GameConfig.wordColr and GameConfig.scrnColr.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfig_wordColr_scrnColr(GameConfig *config) {
    printf("查看颜色表，输入1000\n");
    int wordColr=-1;

    do {
        printf("输入文字颜色对应的数字(0~255)：");
        fflush(stdout);
        scanf("%d",&wordColr);
        while ( getchar()!='\n' );

        if ( config->scrnColr==wordColr ) {
            printf("背景颜色不能与文字颜色相同\n");
            wordColr=-1;
            continue;
        } else if ( wordColr<=255 && wordColr>=0 ) {
            config->wordColr=wordColr;
            printf("\033[38;5;%dm",wordColr);
        } else if ( wordColr==1000 ){
            terminal256ColorTablePainting(config);
            printf("查看颜色表，输入1000\n");
            wordColr=-1;
            continue;
        }
        break;
    } while ( 1 );

    do {
        printf("输入背景颜色对应的数字(0~255)：");
        fflush(stdout);
        int screenColor=-1;

        scanf("%d",&screenColor);
        while ( getchar()!='\n' );

        if ( screenColor==wordColr ) {
            printf("文字颜色不能与背景颜色相同\n");
            screenColor=-1;
            continue;
        } else if ( screenColor<=255 && screenColor>=0 ) {
            config->scrnColr=screenColor;
            printf("\033[48;5;%dm",screenColor);
        } else if ( screenColor==1000 ) {
            terminal256ColorTablePainting(config);
            printf("查看颜色表，输入1000\n");
            screenColor=-1;
            continue;
        }
        break;
    } while ( 1 );
}

/**
 * @brief Restore all settings to default.
 *
 * @param[in,out] config Game configuration of program reading
 *                       and writing configuration files.
 */
void setConfigAllToDefault(GameConfig* config) {
    Point termSize=terminalSize();

    config->wallNum=0;
    config->foodNum=1;

    config->isEnableObs=0;
    config->isEnableEatSlfGmOver=0;

    config->speed=450000u;
    config->minScrOpnVctryPnt=5;

    config->wordColr=0;
    config->scrnColr=231;

    config->scrnHigh=HIGH=termSize.x/2-3;
    config->scrnWide=WIDE=termSize.y;
}
