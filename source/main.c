/**
 * @file main.c
 * @brief This source realizes the @ref main function.
 * @author Zhang Zhiyu
 */

#include "GSnakeBInclude/Functions/painting.h"
#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/userInterfaceBeforeGameStarts.h"
#include "GSnakeBInclude/Functions/gameApplicationStartupRelated.h"
#include "GSnakeBInclude/Functions/gameStartupRelated.h"
#include "GSnakeBInclude/Functions/signalCapture.h"
#include "GSnakeBInclude/Functions/gameMainLogic.h"
#include "GSnakeBInclude/Functions/standardIO.h"
#include "GSnakeBInclude/Functions/terminal.h"
#include "GSnakeBInclude/Functions/exitApp.h"

#include <linux/prctl.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc,char* argv[]) {
    int fd[2]={0};
    int ret=pipe(fd);
    if (ret==-1) {
        perror("pipe error");
        printf("错误，游戏无法启动，正在退出！\n");
        exit(1);
    }

    pid_t pid=fork();
    if (pid==-1) {
        perror("fork error");
        printf("错误，游戏无法启动，正在退出！\n");
        exit(1);
    } else if (pid==0) {
        initTerminalSettings();

        close(fd[0]);
        prctl(PR_SET_PDEATHSIG,SIGINT);

        struct sigaction saINT;
        saINT.sa_handler = &childProcessCatchINT;
        sigemptyset(&saINT.sa_mask);
        saINT.sa_flags = 0;
        if (sigaction(SIGINT, &saINT, NULL) == -1) {
            perror("sigaction error");
            printf("错误，游戏无法启动，正在退出！\n");
            return 1;
        }
        
        struct sigaction saUSR1;
        saUSR1.sa_handler = &childProcessCatchUSR1;
        sigemptyset(&saUSR1.sa_mask);
        saUSR1.sa_flags = 0;
        if (sigaction(SIGUSR1, &saUSR1, NULL) == -1) {
            perror("sigaction error");
            printf("错误，游戏无法启动，正在退出！\n");
            return 1;
        }
        
        struct sigaction saUSR2;
        saUSR2.sa_handler = &childProcessCatchUSR2;
        sigemptyset(&saUSR2.sa_mask);
        saUSR2.sa_flags = 0;
        if (sigaction(SIGUSR2, &saUSR2, NULL) == -1) {
            perror("sigaction error");
            printf("错误，游戏无法启动，正在退出！\n");
            return 1;
        }

        char buf[4096];
        int ret=0;
        while (1) {
            ret=read(STDIN_FILENO,buf,4096);
            
            for (int i=0; i<ret; ++i) {
                if (buf[i]=='e' || buf[i]=='E') {
                    kill(getppid(),SIGINT);
                    close(fd[1]);
                    while (1);
                }
            }

            write(fd[1],buf,ret);
        }
    } else if (pid>0) {
        close(fd[1]);
        dup2(fd[0],STDIN_FILENO);

        struct sigaction saINT;
        saINT.sa_handler = &fatherProcessCatchINT;
        sigemptyset(&saINT.sa_mask);
        saINT.sa_flags = 0;

        if (sigaction(SIGINT, &saINT, NULL) == -1) {
            perror("sigaction error");
            printf("错误，游戏无法启动，正在退出！\n");
            exit(1);
        }

        struct sigaction saCHLD;
        saCHLD.sa_handler = &fatherProcessCatchCHLD;
        sigemptyset(&saCHLD.sa_mask);
        saCHLD.sa_flags = 0;
        
        if (sigaction(SIGCHLD, &saCHLD, NULL) == -1) {
            perror("sigaction error");
            printf("错误，游戏无法启动，正在退出！\n");
            exit(1);
        }
        
        GameAllRunningData *data=malloc(sizeof(GameAllRunningData));
        if ( data==NULL )
        {
            perror("malloc error");
            printf("空间申请失败，游戏无法开启！\n");
            return 1;
        }

        printf("\033[?25l"); // 隐藏光标
        clearScreen();

        printf("\033[48;5;");
        gameStartupLoading();
        int initRet=confgFileInitAndGameIntrfcRndrng();
        if ( initRet==1 )
        {
            firstLoginToGameLoading();
            printf("欢迎您体验贪吃蛇大作战游戏，在体验之前，您需要了解");
            gameIntroduction();
        }
        else if ( initRet==-1 )
        {
            isConfigFileOpenFail=true;
        }

        do {
            enableNormalInput(pid);

            memset(data,0,sizeof(GameAllRunningData));
            if ( showGameMenu(data,pid) ) break;
            
            initAllGameData(data);
            
            wallPainting(data);
            gameInterfacePainting(data);
            
            mainLoopOfGameProcess(data);
        } while ( 1 );

        exitApp(0,data);
        return 0;
    }
}
