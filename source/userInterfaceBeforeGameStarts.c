/**
 * @file userInterfaceBeforeGameStarts.c
 * @brief This source realizes the functions about user
 *        interface before the game starts.
 */

#include "GSnakeBInclude/Struct/GameAllRunningData.h"
#include "GSnakeBInclude/Struct/GameConfig.h"
#include "GSnakeBInclude/Struct/Point.h"
#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/standardIO.h"
#include "GSnakeBInclude/Functions/terminal.h"
#include "GSnakeBInclude/Functions/painting.h"

#include <stdlib.h>
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

    clearScreen();
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
    FILE* fp=NULL;
    char* entry[] = {
        "蛇吃到自己是否退出********",
        "蛇移一格需要的秒数********",
        "开启胜利点所需积分********",
        "食物的初始化的数量********",
        "围墙的初始化的数量********",
        "是否开启系统的小蛇********",
        "所有设置一键初始化********",
        "设置游戏界面的宽高********",
        "设置游戏的背景颜色********",
        "退出游戏的设置界面********",
    };
    char tip[256] = { 0 };

    if ( isConfigFileOpenFail==false ) {
        fp=fopen("./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data","r");
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

    HIGH=config.scrnHigh;
    WIDE=config.scrnWide;

    for ( muint_t setOptionCode=1; setOptionCode != 10; ) {
        tip[0] = '\0';
        setOptionCode = selectInterfacePainting(10, entry, setOptionCode);

        switch (setOptionCode) {
        case 1:
            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(1->YES):", entry[0]);
            config.isEnableEatSlfGmOver = adjustNumberPainting(
                                            config.isEnableEatSlfGmOver,
                                            0, 1, 0, tip, "", config);
            break;

        case 2:
            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(0.2-2s):", entry[1]);
            config.speed = 1000 * 1000 * adjustNumberPainting(
                                           (double)config.speed / 1000.0 / 1000.0,
                                           2, 2.0, 0.2, tip, "", config);
            config.speed -= config.speed % 10000;
            break;

        case 3:
            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(30-500):", entry[2]);
            config.minScrOpnVctryPnt = adjustNumberPainting(
                                         config.minScrOpnVctryPnt,
                                         0, 50, 3, tip, "0", config);
            break;

        case 4:
            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(1-20):", entry[3]);
            config.foodNum = adjustNumberPainting(
                               config.foodNum,
                               0, 20, 1, tip, "", config);
            break;

        case 5:
            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(0-15):", entry[4]);
            config.wallNum = adjustNumberPainting(
                               config.wallNum,
                               0, 15, 0, tip, "", config);
            break;

        case 6:
            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(1->YES):", entry[5]);
            config.isEnableObs = adjustNumberPainting(
                                   config.isEnableObs,
                                   0, 1, 0, tip, "", config);
            break;

        case 7:
            {
                Point termSize=terminalSize();

                config.wallNum=0;
                config.foodNum=1;

                config.isEnableObs=0;
                config.isEnableEatSlfGmOver=0;

                config.speed=450000u;
                config.minScrOpnVctryPnt=5;

                config.wordColr=0;
                config.scrnColr=231;

                config.scrnHigh=HIGH=termSize.x-5;
                config.scrnWide=WIDE=termSize.y;
            }
            break;

        case 8:
            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(10-%d):", entry[7], terminalSize().y);
            config.scrnWide = adjustNumberPainting(
                                config.scrnWide,
                                0, terminalSize().y, 10, tip, " (当前设置:宽)", config);

            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(5-%d):", entry[7], terminalSize().x - 5);
            config.scrnHigh = adjustNumberPainting(
                                config.scrnHigh,
                                0, terminalSize().x - 5, 5, tip, " (当前设置:高)", config);
            break;

        case 9:
            snprintf(tip, sizeof(tip), "> %s\b\b\b\b\b\b\b\b(0-255):", entry[8]);

            do {
                config.scrnColr = adjustNumberPainting(
                                    config.scrnColr,
                                    0, 256, 0, tip, " (当前设置:背景,输入256查看颜色表)", config);
                if (config.scrnColr == 256) {
                    terminal256ColorTablePainting(&config);

                    clearScreen();
                    for (int i = 0; i < 10; ++i) {
                        printf("  %s\n", entry[i]);
                    }
                    printf("\033[9;1H");
                    fflush(stdout);
                }
            } while (config.scrnColr == 256);

            do {
                config.wordColr = adjustNumberPainting(
                                    config.wordColr,
                                    0, 256, 0, tip, " (当前设置:文字,输入256查看颜色表)", config);
                if (config.wordColr == 256) {
                    terminal256ColorTablePainting(&config);

                    clearScreen();
                    for (int i = 0; i < 10; ++i) {
                        printf("  %s\n", entry[i]);
                    }
                    printf("\033[9;1H");
                    fflush(stdout);
                }
            } while (config.wordColr == 256);
            break;
        }
    }

    if ( isConfigFileOpenFail==false ) {
        fp=fopen("./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data","w");
        if ( fp==NULL ) {
            printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");

            isConfigFileOpenFail=true;
            outlineModeConfig=config;

            blockWaitUserEnter();
            clearScreen();
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
    char* entry[] = {
        "*************开始游戏**************",
        "*************说明游戏**************",
        "*************设置游戏**************",
        "*************结束游戏**************",
    };
    clearScreen();

    for ( muint_t menuOptionCode=0; menuOptionCode!=1; ) {
        menuOptionCode = selectInterfacePainting(4, entry, menuOptionCode);

        switch (menuOptionCode) {
        case 1:
            break;

        case 2:
            gameIntroduction();
            break;

        case 3:
            configureGame();
            break;

        case 4:
            return true;
        }
    }
        
    printf("\033[5;1H马上开始游戏");
    srand((unsigned)time(0));
    for (int i=0,totalTimes=rand()%9+3; i<totalTimes; ++i) {
        printf(".");
        fflush(stdout);
        usleep(300*1000u);
    }
    clearScreen();
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
    clearScreen();
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
