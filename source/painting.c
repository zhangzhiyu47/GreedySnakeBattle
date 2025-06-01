/**
 * @file painting.c
 * @brief This source realizes the functions about painting interface.
 */

#include "GSnakeBInclude/Struct/GameAllRunningData.h"
#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/standardIO.h"
#include "GSnakeBInclude/Functions/terminal.h"
#include "GSnakeBInclude/Struct/GameConfig.h"
#include "GSnakeBInclude/Functions/painting.h"

#include <stdbool.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#define MAX_DIGITS 128

/**
 * Formats number with leading zeros and fixed decimal places
 * 
 * @param value Number to format
 * @param decimalPlaces Number of decimal places
 * @param maxValue Maximum value (determines leading zeros)
 * @param buffer Output buffer
 * @param bufferSize Buffer size
 */
void formatNumber(double value, int decimalPlaces, double maxValue, 
                  char* buffer, size_t bufferSize) {
    // Calculate required integer digits
    int maxIntDigits = (maxValue >= 1) ? (int)log10(maxValue) + 1 : 1;
    
    // Format with leading zeros
    if (decimalPlaces > 0) {
        snprintf(buffer, bufferSize, "%0*.*f", 
                maxIntDigits + 1 + decimalPlaces, // +1 for decimal point
                decimalPlaces, value);
    } else {
        snprintf(buffer, bufferSize, "%0*d", maxIntDigits, (int)round(value));
    }
    
    // Ensure we don't have trailing . (e.g. 100. -> 100)
    char* dot = strchr(buffer, '.');
    if (dot && dot[1] == '\0') {
        *dot = '\0';
    }
}

/**
 * Adjusts number with full cursor control and validation
 * 
 * @param defaultValue Initial value
 * @param decimalPlaces Decimal places
 * @param maxValue Maximum value
 * @param minValue Minimum value
 * @param prefixText Prefix text
 * @param suffixText Suffix text
 * @param config Game config
 * @return Final adjusted value
 */
double adjustNumberPainting(double defaultValue, unsigned int decimalPlaces, 
                    double maxValue, double minValue,
                    const char* prefixText, const char* suffixText, const GameConfig config) {
    struct termios oldTerm, newTerm;
    char numStr[MAX_DIGITS] = {0};
    int cursorPos = 0;
    bool editing = true;
    double currentValue = defaultValue;
    
    // Validate parameters
    if (currentValue > maxValue) currentValue = maxValue;
    if (currentValue < minValue) currentValue = minValue;
    
    // Initialize number string with leading zeros
    formatNumber(currentValue, decimalPlaces, maxValue, numStr, MAX_DIGITS);
    cursorPos = strlen(numStr) - 1; // Start at last digit
    
    while (editing) {
        // Display number with cursor
        printf("\r\033[K%s", prefixText);
        for (int i = 0; i < strlen(numStr); i++) {
            if (i == cursorPos) {
                printf("\033[4m\033[5m\033[1m\033[7m%c\033[0m", numStr[i]); // Underline cursor position
                printf("\033[?25l");
                printf("\033[48;5;%dm", config.scrnColr);
                printf("\033[38;5;%dm", config.wordColr);
            } else {
                putchar(numStr[i]);
            }
        }
        printf("%s", suffixText);
        fflush(stdout);
        
        char c = getchar();
        if (c == '\e' && getchar() == '[') { // Arrow key sequence
            c = getchar();

            switch (c) {     // Change ASCII code escape characters into input W,A,S,D
                case 'A':
                    c = 'w';
                    break;
                case 'B':
                    c = 's';
                    break;
                case 'C':
                    c = 'd';
                    break;
                case 'D':
                    c = 'a';
                    break;
            }
        }
        
        switch (c) {
            case 'a': // Left
            case 'A': // Left
                if (cursorPos > 0) {
                    cursorPos--;
                    if (numStr[cursorPos] == '.') cursorPos--;
                    // Add leading zero if at start
                    if (cursorPos < 0) {
                        memmove(numStr+1, numStr, strlen(numStr)+1);
                        numStr[0] = '0';
                        cursorPos = 0;
                    }
                }
                break;
                
            case 'd': // Right
            case 'D': // Right
                if (cursorPos < strlen(numStr)-1) {
                    cursorPos++;
                    if (numStr[cursorPos] == '.') cursorPos++;
                }
                break;
                
            case 'w': // Up
            case 'W': // Up
                if (cursorPos < strlen(numStr) && isdigit(numStr[cursorPos])) {
                    if (numStr[cursorPos] < '9') {
                        numStr[cursorPos]++;
                    } else {
                        numStr[cursorPos] = '0';
                        // Handle carry
                        for (int i = cursorPos-1; i >= 0; i--) {
                            if (numStr[i] == '.') continue;
                            if (numStr[i] < '9') {
                                numStr[i]++;
                                break;
                            } else {
                                numStr[i] = '0';
                                // Add new digit if needed
                                if (i == 0) {
                                    memmove(numStr+1, numStr, strlen(numStr)+1);
                                    numStr[0] = '1';
                                    cursorPos++;
                                }
                            }
                        }
                    }
                    // Validate and reformat
                    currentValue = atof(numStr);
                    if (currentValue > maxValue) {
                        currentValue = maxValue;
                    }
                    formatNumber(currentValue, decimalPlaces, maxValue, numStr, MAX_DIGITS);
                    // Adjust cursor position
                    if (cursorPos >= strlen(numStr)) {
                        cursorPos = strlen(numStr)-1;
                    }
                }
                break;
                
            case 's': // Down
            case 'S': // Down
                if (cursorPos < strlen(numStr) && isdigit(numStr[cursorPos])) {
                    if (numStr[cursorPos] > '0') {
                        numStr[cursorPos]--;
                    } else {
                        numStr[cursorPos] = '9';
                        // Handle borrow
                        for (int i = cursorPos-1; i >= 0; i--) {
                            if (numStr[i] == '.') continue;
                            if (numStr[i] > '0') {
                                numStr[i]--;
                                break;
                            } else {
                                numStr[i] = '9';
                            }
                        }
                    }
                    // Validate and reformat
                    currentValue = atof(numStr);
                    if (currentValue < minValue) {
                        currentValue = minValue;
                    }
                    formatNumber(currentValue, decimalPlaces, maxValue, numStr, MAX_DIGITS);
                    // Adjust cursor position
                    if (cursorPos >= strlen(numStr)) {
                        cursorPos = strlen(numStr)-1;
                    }
                }
                break;
                
            case '\n': // Enter
            case '\t': // Tab
                editing = false;
                break;
        }
    }
    
    // Final validation
    if (currentValue > maxValue) currentValue = maxValue;
    if (currentValue < minValue) currentValue = minValue;
    
    return currentValue;
}

/**
 * @brief Displays a menu interface and allows user to
 *        select an option using arrow keys or w/s keys.
 * 
 * This function prints a list of menu entries and allows
 * the user to navigate through them using either arrow
 * keys (up/down) or w/s keys. The currently selected option
 * is indicated by a > marker. The selection is confirmed
 * by pressing Enter or Tab.
 *
 * @param size The number of entries in the menu
 * @param entry An array of strings representing the menu entries
 * @param defaultOption At the beginning, point to the option
 *                      pointed by the marker >
 * @return The index (1-based) of the selected menu option
 * 
 * @note The function uses ANSI escape codes for cursor positioning
 * @note Navigation can be done with arrow keys (converted
 *       to w/s) or direct w/s key presses
 * @note The selection is confirmed with Enter or Tab key
 */
int selectInterfacePainting(int size, char* entry[], unsigned int defaultOption) {
    int option = 1;
    char c = '\0';

    if (defaultOption >= 1 && defaultOption <= size) {
        option = defaultOption;
    }

    clearScreen();
    
    printf("\033[1;1H");
    for (int i = 0; i < size; ++i) {
        printf("  %s\n", entry[i]);
    }
    printf("\033[%d;1H>", option);
    fflush(stdout);

    c = getchar();
    while (c != '\n' && c != '\t') {
        if (c == '\033' && getchar() == '[') {
            switch (getchar()) {
                case 'A':
                    c = 'w';
                    break;
                case 'B':
                    c = 's';
                    break;
            }
        }

        switch (c) {
            case 'w':
            case 'W':
                printf("\033[%d;1H ", option);

                if (option == 1) {
                    option = size;
                } else {
                    --option;
                }

                printf("\033[%d;1H>", option);
                break;

            case 's':
            case 'S':
                printf("\033[%d;1H ", option);

                if (option == size) {
                    option = 1;
                } else {
                    ++option;
                }

                printf("\033[%d;1H>", option);
                break;

            case 'r':
            case 'R':
                clearScreen();
                printf("\033[1;1H");
                for (int i = 0; i < size; ++i) {
                    printf("  %s\n", entry[i]);
                }
                printf("\033[%d;1H>", option);
                break;
        }

        fflush(stdout);
        c = getchar();
    }

    return option;
}

/**
 * @brief Paint all the walls.
 * 
 * Use @ref HIGH,@ref WIDE,GameAllRunningData.wallNum
 * and GameAllRunningData.wall to paint the outer fences
 * and obstacle walls.
 *
 * @param[in,out] data All the game's data when the game is running.
 *
 * @attention The parameters are not checked, the wrong parameters
 *            will produce unpredictable results or crash.
 */
void wallPainting(GameAllRunningData *data) {
    printf("\033[%d;%dH",1,1);
    for ( int i=0; i<HIGH; i++ ) {
        for ( int j=0; j<WIDE; j++ ) {
            if ( i==HIGH-1 || i==0 || j==0 || j==WIDE-1 ) {
                printf("+");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    for ( int i=0; i<data->wallNum; i++ ) {
        printf("\033[%d;%dH+",data->wall[i].y,data->wall[i].x);
    }
    printf("\033[%d;%dH",HIGH,WIDE+1);
}

/**
 * @brief Paint the game interface when the game is running.
 *
 * Paint foods,user's snake,obstacle snake(if it is enable)
 * and user's score.
 *
 * @param[in] data All the game's data when the game is running.
 */
void gameInterfacePainting(GameAllRunningData const *const data) {
    for ( int i=0; i<data->foodNum; i++ ) {
        printf("\033[%d;%dH",data->food[i].y, data->food[i].x);
        printf("#");
    }
    for ( int i=data->obsSnkLeng-1; i>=0 && \
            data->isEnableObs==1; i-- ) {
        printf("\033[%d;%dH",data->obsSnkBody[i].y, \
               data->obsSnkBody[i].x);
        if ( data->obsState!=0 ) {
            if ( data->obsState==2 ) {
                printf("#");
            } else i?printf("#"):printf("+");
        } else if ( i==0 ) {
            printf("$");
        } else {
            printf("%%");
        }
    }
    for ( int i=data->usrSnkLeng-1; i>=0; i-- ) {
        printf("\033[%d;%dH",data->usrSnkBody[i].y, data->usrSnkBody[i].x);
        if ( i==0 ) {
            printf("@");
        } else {
            printf("*");
        }
    }

    printf("\033[%d;%dH",HIGH+1,1);
    printf("%05d分",data->usrSrc);
    if ( data->usrSrc/10>data->histryHighestScr )
        printf("\033[%d;%dH已超过历史记录!",HIGH+1,9);
    printf("\033[%d;%dH",HIGH, WIDE+1);
}

/**
 * @brief Paint the game-over interface and update the highest
 *        history score(If the configuration file open failed,
 *        it will start the Offline Mode).
 *
 * @param[in,out] data All the game's data when the game is running.
 */
void gameOverPainting(GameAllRunningData *data) {
    printf("\033[%d;%dH",HIGH/2-1,(WIDE-12)/2);
    if ( data->usrSnkGameEndState==1 ) {
        printf("你赢了 太厉害了");
    } else {
        printf("\033[%d;%dH",HIGH/2-1,(WIDE-12)/2);
        printf("你输了 下次注意");
    }
    
    printf("\033[%d;%dH",HIGH+2,(WIDE-20)/2);
    printf("游戏结束 分数为%d\n",data->usrSrc);
    
    if ( data->usrSrc/10>data->histryHighestScr ) {
        printf("你真厉害，创造了新的记录，超过了历史最高记录%d分\n",data->histryHighestScr*10);
        if ( isConfigFileOpenFail==false ) {
            FILE *fp=fopen("GreedySnakeBattleGame.conf","r");
            GameConfig config={0};

            if ( fp==NULL ) {
                printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");

                isConfigFileOpenFail=true;

                outlineModeConfig.wallNum=data->wallNum;
                outlineModeConfig.foodNum=data->foodNum;

                outlineModeConfig.isEnableEatSlfGmOver=data->isEnableEatSlfGmOver;
                outlineModeConfig.isEnableObs=data->isEnableObs;

                outlineModeConfig.histryHighestScr=data->usrSrc/10;
                outlineModeConfig.minScrOpnVctryPnt=data->minScrOpnVctryPnt;
                outlineModeConfig.speed=data->speed;

                outlineModeConfig.scrnHigh=HIGH;
                outlineModeConfig.scrnWide=WIDE;

                blockWaitUserEnter();
                clearScreen();
            } else {
                fread(&config,sizeof(GameConfig),1,fp);
                config.histryHighestScr=data->usrSrc/10;
                fclose(fp);
                fp=NULL;

                fp=fopen("GreedySnakeBattleGame.conf","w");

                if ( fp==NULL ) {
                    printf("配置文件打开失败，马上开启离线模式(您的设置信息可能会丢失)\n");

                    isConfigFileOpenFail=true;

                    outlineModeConfig=config;
                    outlineModeConfig.histryHighestScr=data->usrSrc/10;

                    blockWaitUserEnter();
                    clearScreen();
                } else {
                    fwrite(&config,sizeof(GameConfig),1,fp);
                    fclose(fp);
                }
            }
        } else {
            outlineModeConfig.histryHighestScr=data->usrSrc/10;
        }
    }

    if ( data->usrSnkIsEatingObsSnk ) {
        printf("你真厉害，你吃到了系统小蛇\n");
    }

    blockWaitUserEnter();
}
