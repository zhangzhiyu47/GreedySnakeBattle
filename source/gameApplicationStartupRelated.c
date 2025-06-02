/**
 * @file gameApplicationStartupRelated.c
 * @brief This source realizes the functions about launching game app.
 */

#include "GSnakeBInclude/Struct/GameConfig.h"
#include "GSnakeBInclude/Struct/Point.h"
#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/terminal.h"
#include "GSnakeBInclude/Functions/standardIO.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
 * configFile.)
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

    FILE *fp=fopen(configFile,"r");
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
        clearScreen();
        
        HIGH=config.scrnHigh;
        WIDE=config.scrnWide;
        
        fclose(fp);
    } else {
        fp=NULL;

        fp=fopen(configFile,"w");
        printf("\033[48;5;15m");
        printf("\033[38;5;10m");
        for (int i=0; i<termSize.x; ++i) {
            for (int j=0; j<termSize.y; ++j) {
                printf(" ");
            }
        }
        printf("Loading...\n");
        clearScreen();

        if ( fp==NULL ) {
            outlineModeConfig.wallNum=0;
            outlineModeConfig.foodNum=1;

            outlineModeConfig.isEnableEatSlfGmOver=0;
            outlineModeConfig.isEnableObs=0;

            outlineModeConfig.speed=450000u;
            outlineModeConfig.minScrOpnVctryPnt=5;
            outlineModeConfig.histryHighestScr=0;

            outlineModeConfig.wordColr=0;
            outlineModeConfig.scrnColr=231;

            WIDE=outlineModeConfig.scrnWide=termSize.y;
            HIGH=outlineModeConfig.scrnHigh=termSize.x-5;

            printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");
            blockWaitUserEnter();
            clearScreen();
            ret=-1;
        } else {
            GameConfig config= {0};

            config.foodNum=1;
            config.wallNum=0;

            config.isEnableObs=0;
            config.isEnableEatSlfGmOver=0;

            config.speed=450000u;
            config.minScrOpnVctryPnt=5;
            config.histryHighestScr=0;

            HIGH=config.scrnHigh=termSize.x-5;
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

/**
 * @brief Creates application directories including
 *        config and log directories.
 * 
 * Follows XDG Base Directory Specification:
 * - Uses $XDG_CONFIG_HOME/GreedySnakeBattle/ if set
 * - Falls back to ~/.config/GreedySnakeBattle/ otherwise
 * - Creates log/ subdirectory within the config directory
 * 
 * @return 0 on success, -1 on failure with errno set
 *         appropriately
 */
int createAppDirectories() {
    // Get XDG_CONFIG_HOME or fallback to ~/.config
    char* xdgConfigHome = getenv("XDG_CONFIG_HOME");
    char configBasePath[1024] = {0};
    
    if (xdgConfigHome == NULL || xdgConfigHome[0] == '\0') {
        char* home = getenv("HOME");
        if (home == NULL) {
            return -1;
        }
        snprintf(configBasePath, sizeof(configBasePath), "%s/.config", home);
    } else {
        strncpy(configBasePath, xdgConfigHome, sizeof(configBasePath) - 1);
    }

    // Create main application config directory
    snprintf(configDir, sizeof(configDir), "%s/GreedySnakeBattle", configBasePath);
    
    struct stat st;
    if (stat(configDir, &st) == -1) {
        if (mkdir(configDir, 0755) == -1) {
            return -1;
        }
    }

    // Create log subdirectory
    char logPath[2048] = {0};
    snprintf(logPath, sizeof(logPath), "%s/log", configDir);
    
    if (stat(logPath, &st) == -1) {
        if (mkdir(logPath, 0755) == -1) {
            return -1;
        }
    }

    snprintf(configFile, sizeof(configFile), "%s/game.conf", configDir);
    snprintf(logFile, sizeof(logFile), "%s/game.log", logPath);

    return 0;
}
