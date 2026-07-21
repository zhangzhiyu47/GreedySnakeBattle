#include "include/gameConfig.h"
#include "include/Functions/terminal.h"
#include "include/GlobalVariable/globalVariable.h"
#include "include/logger.h"
#include "include/constants.h"
#include "include/Functions/exitApp.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024

static int getBoolConfig(FILE *fp, const char *name, bool *val) {
    char scan[BUFFER_SIZE] = {0};
    char valStr[8] = {0};

    snprintf(scan, BUFFER_SIZE, " %s=%%s", name);

    int retval = fscanf(fp, scan, valStr);

    if (strncmp(valStr, "true", 8) == 0) {
        *val = true;
    } else {
        *val = false;
    }

    return retval;
}

static const char * boolToString(bool val) {
    if (val) {
        return "true";
    } else {
        return "false";
    }
}

/**
 * Get game config, fallback to outline mode
 *
 * @retval 0 success, store game config to parameter config
 *         1 fail to read, but write default config successfully
 */
int getGameConfig(GameConfig *config) {
    FILE *fp = fopen(configFile, "r");
    int ret = 0;

    if ( fp!=NULL ) {
        fscanf(fp, " wallNum=%lu", &config->wallNum);
        fscanf(fp, " foodNum=%lu", &config->foodNum);

        getBoolConfig(fp, "isEnableObs", &config->isEnableObs);
        getBoolConfig(fp, "isEnableEatSlfGmOver", &config->isEnableEatSlfGmOver);

        fscanf(fp, " speed=%lu", &config->speed);
        fscanf(fp, " histryHighestScr=%lu", &config->histryHighestScr);

        fscanf(fp, " scrnHigh=%lu", &config->scrnHigh);
        fscanf(fp, " scrnWide=%lu", &config->scrnWide);

        fscanf(fp, " obsIQ=%lu", &config->obsIQ);

        HIGH=config->scrnHigh;
        WIDE=config->scrnWide;

        fclose(fp);
    } else {
        fp=fopen(configFile, "w");

        if ( fp==NULL ) {
            logger(LOG_ERROR, "fopen: %s" HERE, strerror(errno));
            exitApp(1, "游戏出错，配置文件读取失败！", NULL);
        } else {
            Point termSize = terminalSize();

            fprintf(fp, "wallNum=0\n");
            fprintf(fp, "foodNum=1\n");

            fprintf(fp, "isEnableObs=false\n");
            fprintf(fp, "isEnableEatSlfGmOver=false\n");

            fprintf(fp, "speed=450000\n");
            fprintf(fp, "histryHighestScr=0\n");

            fprintf(fp, "scrnHigh=%lu\n", (uint64_t)termSize.y);
            fprintf(fp, "scrnWide=%lu\n",
                    (uint64_t)termSize.x - ROCKER_BAR_WIDTH);

            fprintf(fp, "obsIQ=3\n");

            fclose(fp);

            config->foodNum=1;
            config->wallNum=0;

            config->isEnableObs=0;
            config->isEnableEatSlfGmOver=0;

            config->speed=450000u;
            config->histryHighestScr=0;

            HIGH=config->scrnHigh=termSize.y;
            WIDE=config->scrnWide=termSize.x - ROCKER_BAR_WIDTH;

            config->obsIQ = 3;

            ret = 1;
        }
    }

    return ret;
}

/// Write game config file
int setGameConfig(GameConfig *config) {
    int ret = 0;

    FILE *fp=fopen(configFile, "w");

    if (fp == NULL) {
        logger(LOG_ERROR, "fopen: %s" HERE, strerror(errno));
        exitApp(1, "游戏出错，配置文件写入失败！", NULL);
    } else {
        fprintf(fp, "wallNum=%lu\n", config->wallNum);
        fprintf(fp, "foodNum=%lu\n", config->foodNum);

        fprintf(fp, "isEnableObs=%s\n", boolToString(config->isEnableObs));
        fprintf(fp, "isEnableEatSlfGmOver=%s\n",
                boolToString(config->isEnableEatSlfGmOver));

        fprintf(fp, "speed=%lu\n", config->speed);
        fprintf(fp, "histryHighestScr=%lu\n", config->histryHighestScr);

        fprintf(fp, "scrnHigh=%lu\n", config->scrnHigh);
        fprintf(fp, "scrnWide=%lu\n", config->scrnWide);

        fprintf(fp, "obsIQ=%lu\n", config->obsIQ);

        fclose(fp);
    }

    return ret;
}
