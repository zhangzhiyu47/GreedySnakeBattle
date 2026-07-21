#include "include/Struct/GameAllRunningData.h"
#include "include/Functions/terminal.h"
#include "include/Functions/exitApp.h"
#include "include/GlobalVariable/globalVariable.h"
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
    printf("\033[?25h\033[0m\033[?1049l\033[?1002;1006l");
    if (strlen(tip)) {
        printf("%s\n", tip);
    }
    fflush(stdout);

    if (retn) {
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
    restoreTerminalSettings();

    ftruncate(lockFileFd, 0);
    close(lockFileFd);

    exit(retn);
}
