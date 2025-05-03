/**
 * @file userInterfaceBeforeGameStarts.c
 * @brief This source realizes the functions about user
 *        interface before the game starts.
 */

#include "Include/Compat/snakeFullCompat.h"
#include "Include/Struct/GameAllRunningData.h"
#include "Include/Struct/GameConfig.h"
#include "Include/Struct/Point.h"
#include "Include/GlobalVariable/globalVariable.h"
#include "Include/Functions/gameSetConfiguration.h"
#include "Include/Functions/standardIO.h"
#include "Include/Functions/terminal.h"
#include <stdio.h>
#include <time.h>

/**
 * @brief Introduce and explain the game in detail.
 *
 * Introduce and explain the game in detail (basic gameplay,
 * game interface introduction, game rules and offline mode).
 */
void gameIntroduction() {
    const char * gameIntroText=
        "贪吃蛇大作战简介：\n"
        "    贪吃蛇大作战是一款经典的游戏，我通过简单的c语言制作出的游戏模型，"
        "你可以通过\"W，A，S，D\"键分别控制蛇上左下右移动（触发神秘键会有惊喜）。"
        "无论何时输入\"E\"会直接退出游戏，在游戏中输入\"O\"会退出当前的对局。"
        "在选择界面时，输入功能前的序号来进行操作，输入错误时系统会等待您输入正确的数字\n"
        "    \"+\"为墙，\"#\"为食物（障碍蛇死后也会变为食物），\"@\"为蛇头，\"*\"为蛇身，\"$\"为胜利点。"
        "蛇头碰到墙(或蛇身)会死亡，变为食物，"
        "吃到食物会增加分数和蛇长，当分数到达一定量时，"
        "会显示胜利点，蛇头到达就会胜利，或当到达一定分数时会自动胜利。\n"
        "    若对游戏设置有所不满，请在\"设置游戏\"中自行设置。\n"
        "离线模式：\n"
        "    当配置文件无法打开时，该模式将被打开。有许多地方会读取配置文件，其无法读取的原因有很多。"
        "一旦离线模式已打开，在游戏开始之前无法关闭结束。"
        "在此模式下，游戏不再读取配置文件，但依赖于其自己的配置变量和默认值配置进行游戏。\n"
        "    注意！在这种模式下仍然可以调整游戏设置，但在不会被保存的配置文件中，"
        "设置只能持续到游戏结束，并且不能自动恢复玩家设置的内容，不建议在该模式下更改游戏设置。\n"
    ;

    clearAll();
    printf("%s",gameIntroText);
    blockWaitUserEnter();
}

/**
 * @brief Configure the game.
 *
 * Read the game settings adjusted by the end user and save
 * them to the configuration file for use when loading the
 * game interface. Due to file read and write operations,
 * offline mode may be enabled. In offline mode, the game can
 * still be configured (see @ref OffLineMode for details).
 */
void configureGame() {
    GameConfig config={0};
    FILE *fp=NULL;

    if ( isConfigFileOpenFail==false ) {
        fp=fopen("./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data","r");
        if ( fp==NULL ) {
            printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");
            isConfigFileOpenFail=true;
            config=outlineModeConfig;
        } else {
            fread(&config,sizeof(GameConfig),1,fp);
            fclose(fp);
            fp=NULL;
        }
    } else {
        config=outlineModeConfig;
    }

    HIGH=config.scrnHigh;
    WIDE=config.scrnWide;

    for ( muint_t setOptionCode=2; setOptionCode<=10 && setOptionCode>=1; ) {
        setOptionCode=0;
        if ( isConfigFileOpenFail) {
            printf("(当前是离线模式)\n");
        }
        
        printf("01：蛇吃到自己是否退出：%s\n",config.isEnableEatSlfGmOver?"是":"否");
        printf("02：蛇移一格需要的秒数：%.2f\n",config.speed/1000000.0);
        printf("03：开启胜利点所需积分：%d\n",config.minScrOpnVctryPnt*10);
        printf("04：食物的初始化的数量：%d\n",config.foodNum);
        printf("05：围墙的初始化的数量：%d\n",config.wallNum);
        printf("06：是否开启系统的小蛇：%s\n",config.isEnableObs?"是":"否");
        printf("07：所有设置一键初始化：无参数\n08：贪吃蛇历史最高记录：%d分\n",config.histryHighestScr*10);
        printf("09：设置游戏界面的长宽：垂直长度：%d  水平长度：%d\n",HIGH,WIDE);
        printf("10：设置游戏的背景颜色：字体：%d  背景：%d\n",config.wordColr,config.scrnColr);
        printf("注：输入错误的的数值将不改变原来的数值\n");
        printf("请输入你要修改的号码（输入其他将直接退出）：");
        fflush(stdout);

        scanf("%u",&setOptionCode);
        while ( getchar()!='\n' );

        switch ( setOptionCode ) {
        case 1:
            setConfig_isEnableEatSlfGmOver(&config);
            break;
        case 2:
            setConfig_speed(&config);
            break;
        case 3:
            setConfig_minScrOpnVctryPnt(&config);
            break;
        case 4:
            setConfig_foodNum(&config);
            break;
        case 5:
            setConfig_wallNum(&config);
            break;
        case 6:
            setConfig_isEnableObs(&config);
            break;
        case 7:
            setConfigAllToDefault(&config);
            break;
        case 8:
            break;
        case 9:
            setConfig_scrnHigh_scrnWide(&config);
            break;
        case 10:
            setConfig_wordColr_scrnColr(&config);
            break;
        }
        clearAll();
    }

    if ( isConfigFileOpenFail==false ) {
        fp=fopen("./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data","w");
        if ( fp==NULL ) {
            printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");
            isConfigFileOpenFail=true;
            outlineModeConfig=config;
        } else {
            fwrite(&config,sizeof(GameConfig),1,fp);
            fclose(fp);
        }
    }
    else {
        outlineModeConfig=config;
    }
}

/**
 * @brief Show game menu.
 *
 * Let users choose to start the game, introduce the game,
 * set the game and exit the game. And perform the
 * corresponding behavior. In addition to starting the game
 * and exiting the game, the other two options will print the
 * menu again after execution.
 * | option | action |
 * | :--: | :----: |
 * | 1 | Start the game |
 * | 2 | Introduce the game |
 * | 3 | set the game |
 * | 4 | Exit the game |
 *
 * @param[in,out] data All the game's data when the game is running.
 * @param[in] childProcess ID of the child process.
 *
 * @return Does user want to quit the game?
 * @retval true  Yes.Then quit the game.
 * @retval false No.Then start the game.
 */
bool showGameMenu(GameAllRunningData *data,const pid_t childProcess) {
    clearAll();

    for ( muint_t menuOptionCode=0; menuOptionCode!=1; ) {
        menuOptionCode=0;
        if ( isConfigFileOpenFail ) {
            printf("(当前是离线模式)\n");
        }
        printf("*************1:开始游戏**************\n");
        printf("*************2:说明游戏**************\n");
        printf("*************3:设置游戏**************\n");
        printf("*************4:结束游戏**************\n");
        printf("请输入提示前的序号：");
        fflush(stdout);

        scanf("%u",&menuOptionCode);
        while ( getchar()!='\n' );

        switch ( menuOptionCode ) {
        case 1:
            break;
        case 2:
            gameIntroduction();
            break;
        case 3:
            clearAll();
            configureGame();
            break;
        case 4:
            return true;
        default:
            clearAll();
        }
    }
    
    disableNormalInput(childProcess);
    
    printf("马上开始游戏");
    srand((unsigned)time(0));
    for (int i=0,totalTimes=rand()%9+3; i<totalTimes; ++i) {
        printf(".");
        fflush(stdout);
        usleep(300*1000u);
    }
    clearAll();
    return false;
}

/**
 * @brief First login game loading.
 * @note It's just for decoration. It's dispensable.
 */
void firstLoginToGameLoading() {
    printf("\n\n\n");
    srand((unsigned)time(0));
    printf("\n\t    游戏初次加载"
           "\n\t    loading....."
           "\n\t    ");

    for ( int i=0; i<12; i++ ) {
        if ( i!=11 ) {
            muint_t sleepTime=rand()%6+3;
            sleepTime*=100*1000u;
            usleep(sleepTime);

            printf("*");
            fflush(stdout);
        }
    }
    printf("*\n\t    输入任意开始");
    fflush(stdout);
    
    while ( getchar()!='\n' );
    clearAll();
}

/**
 * @brief The game starts to load animation.
 * @note It's just for decoration. It's dispensable.
 */
void gameStartupLoading() {
    const Point termSize=terminalSize();
    
    for ( int i=0; i<(termSize.y-18)/2 && termSize.y>18; i++ ) {
        printf(" ");
    }
    fflush(stdout);
    
    printf("客户资源加载中····\n"
           "进度：[   %%] [");
    for ( int i=0; i<termSize.y-15; i++ ) {
        printf("_");
    }
    printf("]\n");

    srand((unsigned)time(0));
    int slowLoading1=rand()%(termSize.y-15-8);
    int slowLoading2=rand()%(termSize.y-15-5);
    int slowLoading3=rand()%(termSize.y-15-0);

    int progress=0;
    for ( int i=0; i<termSize.y-15; i++ )
    {
        printf("\033[2;%dH",i+15);

        int sleepTime=rand()%(2+(i>termSize.y-15-4 || i==slowLoading1 || i==slowLoading2 || i==slowLoading3?4:0))*100*1000u;
        usleep(sleepTime);
        
        progress=100*(i+1)/(termSize.y-15);
        printf("█\033[2;8H%3d",progress);
        fflush(stdout);
    }
    printf("\033[3;1H");
    blockWaitUserEnter();
}
