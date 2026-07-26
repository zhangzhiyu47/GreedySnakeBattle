#include "include/GameAllRunningData.h"
#include "include/terminal.h"
#include "include/exitApp.h"
#include "include/global.h"
#include "include/logger.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <execinfo.h>
#include <sys/time.h>

static void getTimeStr(char *buf, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm tmInfo;
    localtime_r(&tv.tv_sec, &tmInfo);

    strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tmInfo);
    size_t len = strlen(buf);
    snprintf(buf + len, size - len, ".%06ld", tv.tv_usec);
}

/// Exit the application
void exitApp(int retn, const char *tip,
        const GameAllRunningData *data) {

    restoreTerminalSettings();
    if (retn != EXIT_NORMAL) {
        printf("%s\n", tip);
    }
    fflush(stdout);

    if (retn == EXIT_ERROR) {
        FILE *fp = fopen(errSignFile, "w");
        if (fp) {
            char buf[2048] = {0};

            getTimeStr(buf, 2048);
            fprintf(fp, "When: %s\n", buf);

            fclose(fp);
        } else {
            logger(LOG_ERROR, "fopen: %s" HERE, strerror(errno));
        }
    }

    free((void*)data);
    logCleanup();

    ftruncate(lockFileFd, 0);
    close(lockFileFd);

    exit(retn);
}
