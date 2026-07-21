/**
 * @file main.c
 * @brief This source realizes the @ref main function.
 * @author Zhang Zhiyu
 */

#include "include/Functions/painting.h"
#include "include/GlobalVariable/globalVariable.h"
#include "include/gameMenu.h"
#include "include/gameMainLogic.h"
#include "include/Functions/terminal.h"
#include "include/Functions/exitApp.h"
#include "include/Struct/Point.h"
#include "include/constants.h"
#include "include/logger.h"
#include "include/initGameData.h"
#include "include/initializeApp.h"

#include <linux/prctl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdatomic.h>

static void handlerINT(int signum) {
    UNUSED(signum);

    const char *restoreTerm = "\033[?25h\033[0m\033[?1049l";
    write(STDOUT_FILENO, restoreTerm, 18);

    restoreTerminalSettings();

    ftruncate(lockFileFd, 0);
    close(lockFileFd);

    _exit(0);
}

static void handlerWINCH(int signum) {
    UNUSED(signum);
    atomic_store(&needRedraw, true);
}

int main(int argc,char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);

    createAppDirectories();

    logInit();
    setRotation(ROT_DEFAULT);
    setDefaultLogFile(logFile);
    setDefaultXfdAndColor(STDERR_FILENO, true);

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sa.sa_handler = &handlerINT;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        logger(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(1, "游戏出错，无法启动！", NULL);
    }

    sa.sa_handler = &handlerWINCH;
    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        logger(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(1, "游戏出错，无法启动！", NULL);
    }


    Point termSize = terminalSize();
    if ((uint64_t)termSize.x < MIN_TERMINAL_WIDE
            || (uint64_t)termSize.y < MIN_TERMINAL_HIGH) {
        printf("当前终端太小了，放大试试\n"
               "要求至少 %lux%lu，当前 %lux%lu\n",
               MIN_TERMINAL_WIDE,
               MIN_TERMINAL_HIGH, termSize.x, termSize.y);
        return 0;
    }


    GameAllRunningData *data = malloc(sizeof(GameAllRunningData));
    if (data == NULL) {
        logger(LOG_ERROR, "malloc: %s" HERE, strerror(errno));
        printf("游戏无法开启！\n");
        return 1;
    }

    printf("\033[?1049h");
    printf("\033[?25l");
    clearScreen();
    initTerminalSettings();

    checkLockFile();

    GameConfig config = {0};
    if (initializeApp(&config)) {
        if (requestUserAgreeEULA()) {
            remove(configFile);
            exitApp(0, "", data);
        }
        gameIntroduction("开始游戏");
    }

    do {
        memset(data,0,sizeof(GameAllRunningData));
        if (showGameMenu(&config)) {
            break;
        }

        initGameData(data);

        wallPainting(data);
        gameInterfacePainting(data);

        gameMainLogic(data);
    } while ( 1 );

    exitApp(0, "", data);
}
