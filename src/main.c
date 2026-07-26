#include "include/painting.h"
#include "include/global.h"
#include "include/gameMenu.h"
#include "include/gameMainLogic.h"
#include "include/terminal.h"
#include "include/exitApp.h"
#include "include/constants.h"
#include "include/logger.h"
#include "include/initGameData.h"
#include "include/initializeApp.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <locale.h>
#include <stdatomic.h>

static void handlerINT(int signum) {
    UNUSED(signum);

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

    setlocale(LC_ALL, "");

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
        logdual(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错，无法启动！", NULL);
    }

    sa.sa_handler = &handlerWINCH;
    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        logdual(LOG_ERROR, "sigaction: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错，无法启动！", NULL);
    }

    GameAllRunningData *data = malloc(sizeof(GameAllRunningData));
    if (data == NULL) {
        logdual(LOG_ERROR, "malloc: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错，无法启动！", NULL);
    }

    initTerminalSettings();
    clearScreen();

    checkLockFile();

    GameConfig config = {0};
    if (initializeApp(&config)) {
        if (requestUserAgreeEULA("同意协议内容后开始使用本应用")) {
            remove(configFile);
            exitApp(EXIT_NORMAL, "", data);
        }
        gameIntroduction("开始游戏");
    }

    checkNewVersion();

    if (checkEULAUpdate()) {
        if (!requestUserAgreeEULA("协议更新，需要您重新同意协议")) {
            remove(EULAUpdateSign);
        }
    }

    Point termSize = terminalSize();
    if (termSize.x < MIN_TERMINAL_WIDE
            || termSize.y < MIN_TERMINAL_HIGH) {
        if (screenResizePainting()) {
            exitApp(EXIT_NORMAL, "", data);
        }
    }

    srand((unsigned)time(NULL));

    do {
        memset(data, 0, sizeof(GameAllRunningData));

        int retval = showGameMenu(&config);

        if (retval == GAME_MODE_QUIT) {
            break;
        } else if (retval == GAME_MODE_CLASSIC) {
            initGameData(data);

            gameMainLogic(data);
        } else if (retval == GAME_MODE_UNLIMIT_FOOD) {
            initGameDataUnlimitFood(data);

            gameMainLogicUnlimitedMode(data);
        }
    } while (1);

    exitApp(EXIT_NORMAL, "", data);
}
