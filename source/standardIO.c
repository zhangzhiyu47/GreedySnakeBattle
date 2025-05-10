/**
 * @file standardIO.c
 * @brief This source realizes the functions about standard I/O.
 */

#include "GSnakeBInclude/GlobalVariable/globalVariable.h"
#include "GSnakeBInclude/Functions/standardIO.h"
#include "GSnakeBInclude/Functions/terminal.h"
#include <termios.h>
#include <stdio.h>

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
 * @brief Enable normal input mode (echo + line buffering)
 */
void trulyEnableNormalInput() {
    struct termios newt = originalTermios;
    newt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

/**
 * @brief Disable normal input mode (no echo + no line buffering)
 */
void trulyDisableNormalInput() {
    struct termios newt = originalTermios;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
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

/**
 * @brief Provide an interface for the parent process to let
 *        the child process call the @ref trulyDisableNormalInput
 *        function.
 *
 * Due to replacing the standard input of the parent process
 * with the read end of the pipeline and treating the written
 * content of the pipeline written by the child process as
 * the standard input, it no longer has the ability to directly
 * set the standard input. This function is provided to the
 * parent process that sends the **SIGUSR1** signal to the child
 * process **childProcess**, causing the child process to call
 * the @ref trulyDisableNormalInput function, thereby canceling the
 * standard input buffer and echo.
 *
 * @param[in] childProcess ID of the child process.
 */
void disableNormalInput(const pid_t childProcess) {
    kill(childProcess,SIGUSR1);
}

/**
 * @brief Provide an interface for the parent process to let
 *        the child process call the @ref trulyEnableNormalInput
 *        function.
 *
 * Due to replacing the standard input of the parent process
 * with the read end of the pipeline and treating the written
 * content of the pipeline written by the child process as
 * the standard input, it no longer has the ability to directly
 * set the standard input. This function is provided to the
 * parent process, which sends the **SIGUSR2** signal to the child
 * process **childProcess**, causing the child process to call
 * the @ref trulyEnableNormalInput function, thereby enabling the
 * standard input buffer and input echo.
 *
 * @param[in] childProcess ID of the child process.
 */
void enableNormalInput(const pid_t childProcess) {
    kill(childProcess,SIGUSR2);
}
