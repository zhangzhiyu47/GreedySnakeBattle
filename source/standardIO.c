/**
 * @file standardIO.c
 * @brief This source realizes the functions about standard I/O.
 */

#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/standardIO.h"
#include "GSnakeBInclude/Functions/terminal.h"
#include "GSnakeBInclude/Functions/painting.h"
#include "GSnakeBInclude/Struct/GameConfig.h"
#include <termios.h>
#include <stdio.h>
#include <poll.h>

/**
 * @brief Wait for user input with timeout using poll system call.
 * 
 * @param initialTimeoutMinutes Initial timeout in minutes (can
 *        be overridden by user input).
 * @return int 0 if user input detected, 1 if timeout reached, 2
 *             if system call poll(2) return in error.
 */
int waitForUserInputWithPoll(double initialTimeoutMinutes) {
    struct pollfd fds[1];
    int timeoutMs;
    double userTimeoutMinutes;
    GameConfig config = {0};

    if (isConfigFileOpenFail == false) {
        FILE*fp = fopen("GreedySnakeBattleGame.conf", "r");
        if (fp == NULL) {
            printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");

            isConfigFileOpenFail=true;

            config = outlineModeConfig;

            blockWaitUserEnter();
            clearScreen();
        } else {
            fread(&config, sizeof(GameConfig), 1, fp);
            fclose(fp);
            fp = NULL;
        }
    } else {
        config = outlineModeConfig;
    }

    clearScreen();
    
    userTimeoutMinutes = adjustNumberPainting(
                           initialTimeoutMinutes,
                           1, 24 * 60 * 60, 0, "请输入暂停时间(0-86400(24*60*60)minutes):", " (0表示永久等待)", config);

    // Calculate timeout in milliseconds
    if (userTimeoutMinutes == 0) {
        timeoutMs = -1; // Infinite wait
    } else {
        timeoutMs = userTimeoutMinutes * 60 * 1000; // Convert minutes to milliseconds
    }

    printf("\n开始");
    if (timeoutMs == -1) {
        printf("永久等待(按Enter退出等待)\n");
    } else {
        printf("等待%.1lf分钟(按Enter退出等待)\n", userTimeoutMinutes);
    }

    fds[0].fd = STDIN_FILENO;  // stdin
    fds[0].events = POLLIN;    // Wait for input

    int ret = poll(fds, 1, timeoutMs);

    if (ret == -1) {
        perror("poll error");
        return 2;
    } else if (ret == 0) {
        return 1;
    } else {
        if (fds[0].revents & POLLIN) {
            while (getchar() != '\n');
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Block waiting for user input.
 */
void blockWaitUserEnter() {
    printf("按Enter键继续");
    fflush(stdout);
    
    while ( getchar()!='\n' );
    clearScreen();
}

/**
 * @brief Non-blocking keyboard check
 * @return More than 0 if key pressed, less than or equal to 0
 *         otherwise.
 */
int linuxKbhit() {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
}
