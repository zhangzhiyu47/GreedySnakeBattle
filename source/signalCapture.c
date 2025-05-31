/**
 * @file signalCapture.c
 * @brief This source realizes the functions about signal capturing.
 */


#include "GSnakeBInclude/Functions/terminal.h"

#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Father process captures signal **SIGINT**.
 *
 * Exit the game application after the signal **SIGINT** is
 * delivered (only exit the parent process).
 *
 * @param[in] signum The number of the captured signal.
 */
void fatherProcessCatchINT(int signum) {
    printf("\n游戏结束\n");
    printf("\033[?25h\033[0m");
    fflush(stdout);

    exit(0);
}

/**
 * @brief Father process captures signal **SIGCHLD**.
 *
 * Capture the signal **SIGCHLD** and try to recycle the
 * subprocess to judge whether the subprocess is dead.
 * If dead, exit the game application (that is, the parent
 * process).
 *
 * @param[in] signum The number of the captured signal.
 */
void fatherProcessCatchCHLD(int signum) {
    if ( waitpid(-1,NULL,WNOHANG)!=0 ) {
        printf("警告，子进程死亡，游戏出错，正在退出游戏！\n");
        exit(1);
    }
}

/**
 * @brief Child process captures signal **SIGINT**.
 *
 * Exit this child process after the signal **SIGINT** is
 * delivered.
 *
 * @param[in] signum The number of the captured signal.
 */
void childProcessCatchINT(int signum) {
    restoreTerminalSettings();
    exit(0);
}
