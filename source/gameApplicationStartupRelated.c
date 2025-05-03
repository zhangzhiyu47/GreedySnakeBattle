/**
 * @file gameApplicationStartupRelated.c
 * @brief This source realizes the functions about launching game app.
 */

#include "Include/Struct/GameConfig.h"
#include "Include/Struct/Point.h"
#include "Include/GlobalVariable/globalVariable.h"
#include "Include/Compat/snakeFullCompat.h"
#include "Include/Functions/terminal.h"

/**
 * @brief Configuration file initialization and game interface
 *        rendering.
 *
 * Read the configuration file, and if it exists, render the
 * game interface color and assign GameConfig.scrnHigh and
 * GameConfig.scrnWide to @ref HIGH and @ref WIDE. If the opening fails,
 * open it again by writing, then render the interface. If the
 * opening is successful, write the default game configuration
 * to the configuration file; If unsuccessful, do not read or
 * write files and exit the function.(The configuration file is
 * "./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data".)
 *
 * @return Returns the status of file reading and writing.
 * @retval  0 For successful reading of the configuration file
 *                and rendering.
 * @retval  1 For no configuration file but successfully
 *                initialized and rendered configuration file.
 * @retval -1 For that both reading and writing of the
 *                configuration file have failed, and offline mode
 *                needs to be enabled.
 *
 * @attention This function is complex, but necessary!
 */
int confgFileInitAndGameIntrfcRndrng() {
    const Point termSize=terminalSize();

    FILE *fp=fopen("./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data","r");
    int ret=0;

    if ( fp!=NULL ) {
        GameConfig config= {0};
        fread(&config,sizeof(GameConfig),1,fp);

        printf("\033[48;5;%dm",config.scrnColr);
        printf("\033[38;5;%dm",config.wordColr);
        for (int i=0; i<termSize.x; ++i) {
            for (int j=0; j<termSize.y; ++j) {
                printf(" ");
            }
        }
        printf("Loading...\n");
        clearAll();
        
        HIGH=config.scrnHigh;
        WIDE=config.scrnWide;
        
        fclose(fp);
    } else {
        fp=NULL;

        fp=fopen("./.贪_吃_蛇_大_作_战_的_所_有_设_置_信_息_勿_动.data","w");
        printf("\033[48;5;15m");
        printf("\033[38;5;10m");
        for (int i=0; i<termSize.x; ++i) {
            for (int j=0; j<termSize.y; ++j) {
                printf(" ");
            }
        }
        printf("Loading...\n");
        clearAll();

        if ( fp==NULL ) {
            outlineModeConfig.wallNum=0;
            outlineModeConfig.isEnableEatSlfGmOver=0;
            outlineModeConfig.speed=450000u;
            outlineModeConfig.isEnableObs=0;
            outlineModeConfig.minScrOpnVctryPnt=5;
            outlineModeConfig.foodNum=1;
            outlineModeConfig.histryHighestScr=0;
            outlineModeConfig.wordColr=0;
            outlineModeConfig.scrnColr=231;
            WIDE=outlineModeConfig.scrnWide=termSize.y;
            HIGH=outlineModeConfig.scrnHigh=termSize.x/2-3;

            printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");
            ret=-1;
        } else {
            GameConfig config= {0};

            config.wallNum=0;
            config.isEnableEatSlfGmOver=0;
            config.speed=450000u;
            config.isEnableObs=0;
            config.minScrOpnVctryPnt=5;
            config.foodNum=1;
            config.histryHighestScr=0;
            HIGH=config.scrnHigh=termSize.x/2-3;
            WIDE=config.scrnWide=termSize.y;
            config.wordColr=0;
            config.scrnColr=231;

            fwrite(&config,sizeof(GameConfig),1,fp);
            fclose(fp);
            ret=1;
        }
    }
    return ret;
}
