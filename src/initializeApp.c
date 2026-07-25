#include "include/gameConfig.h"
#include "include/Struct/Point.h"
#include "include/global.h"
#include "include/terminal.h"
#include "include/exitApp.h"
#include "include/initializeApp.h"
#include "include/logger.h"
#include "include/gameMenu.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>

/// Initialize configuration file and render background
int initializeApp(GameConfig *config) {
    int ret = getGameConfig(config);
    HIGH = config->scrnHigh;
    WIDE = config->scrnWide;

    resetColor();
    Point termSize = terminalSize();
    for (uint64_t i = 0; i < termSize.y * termSize.x / 4 + 1; ++i) {
        printf("    ");
    }
    clearScreen();

    return ret;
}

/**
 * @brief Creates application directories including
 *        config and log directories.
 * 
 * Follows XDG Base Directory Specification:
 * - Uses $XDG_CONFIG_HOME/gsb/ if set
 * - Falls back to ~/.config/gsb/ otherwise
 */
void createAppDirectories() {
    // Get XDG_CONFIG_HOME, fallback to ~/.config
    char* xdgConfigHome = getenv("XDG_CONFIG_HOME");
    char configBasePath[1024] = {0};
    
    if (xdgConfigHome == NULL) {
        char* home = getenv("HOME");
        snprintf(configBasePath,
                sizeof(configBasePath), "%s/.config", home);
    } else {
        strncpy(configBasePath, xdgConfigHome,
                sizeof(configBasePath) - 1);
    }

    // Create main application config directory
    snprintf(configDir, sizeof(configDir),
            "%s/gsb", configBasePath);
    
    struct stat st;
    if (stat(configDir, &st) == -1) {
        if (mkdir(configDir, 0700) == -1) {
            perror("(" HERE "): mkdir: ");
            exitApp(EXIT_ERROR, "游戏出错，无法创建配置目录", NULL);
        }
    }

    snprintf(configFile, sizeof(configFile),
            "%s/game.ini", configDir);

    snprintf(logFile, sizeof(logFile),
            "%s/error.log", configDir);

    snprintf(errSignFile, sizeof(errSignFile),
            "%s/.error.sign", configDir);

    snprintf(lockFile, sizeof(lockFile),
            "%s/.lock.pid", configDir);

    snprintf(updateSignFile, sizeof(updateSignFile),
            "%s/NEW_VERSION.txt", configDir);

    snprintf(EULAUpdateSign, sizeof(EULAUpdateSign),
            "%s/.EULA.update", configDir);
}

/// Check the application singleton lock file
void checkLockFile() {
    int fd = open(lockFile, O_RDWR | O_CREAT, 0600);
    if (fd == -1) {
        logger(LOG_ERROR, "open: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错！", NULL);
    }

    if (flock(fd, LOCK_EX | LOCK_NB) != 0) {
        close(fd);
        exitApp(EXIT_NORMAL, "当前已有该应用在运行，无法重复启动", NULL);
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        logger(LOG_ERROR, "fstat: %s" HERE, strerror(errno));
        exitApp(EXIT_ERROR, "游戏出错！", NULL);
    }

    if (st.st_size > 0 || access(errSignFile, F_OK) == 0) {
        showErrorLog();
        remove(errSignFile);
    }

    ftruncate(fd, 0);
    dprintf(fd, "%d", getpid());
    fsync(fd);

    lockFileFd = fd;
}

/// Check new version update
void checkNewVersion() {
    if (access(updateSignFile, F_OK) == 0) {
        showNewVersionInfo();
        remove(updateSignFile);
    }
}

/// Check EULA update
bool checkEULAUpdate() {
    if (access(EULAUpdateSign, F_OK) == 0) {
        return true;
    }
    return false;
}
