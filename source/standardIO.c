/**
 * @file standardIO.c
 * @brief This source realizes the functions about standard I/O.
 */

#include "Include/Compat/snakeFullCompat.h"
#include "Include/Functions/standardIO.h"
#include <stdio.h>

/**
 * @brief Block waiting for user input.
 */
void blockWaitUserEnter() {
    printf("按Enter键继续");
    fflush(stdout);
    
    while ( getchar()!='\n' );
    clearAll();
}

/**
 * @brief Provide an interface for the parent process to let
 *        the child process call the @ref disable_normal_input
 *        function.
 *
 * Due to replacing the standard input of the parent process
 * with the read end of the pipeline and treating the written
 * content of the pipeline written by the child process as
 * the standard input, it no longer has the ability to directly
 * set the standard input. This function is provided to the
 * parent process that sends the **SIGUSR1** signal to the child
 * process **childProcess**, causing the child process to call
 * the @ref disable_normal_input function, thereby canceling the
 * standard input buffer and echo.
 *
 * @param[in] childProcess ID of the child process.
 */
void disableNormalInput(const pid_t childProcess) {
    kill(childProcess,SIGUSR1);
}

/**
 * @brief Provide an interface for the parent process to let
 *        the child process call the @ref enable_normal_input
 *        function.
 *
 * Due to replacing the standard input of the parent process
 * with the read end of the pipeline and treating the written
 * content of the pipeline written by the child process as
 * the standard input, it no longer has the ability to directly
 * set the standard input. This function is provided to the
 * parent process, which sends the **SIGUSR2** signal to the child
 * process **childProcess**, causing the child process to call
 * the @ref enable_norms_input function, thereby enabling the
 * standard input buffer and input echo.
 *
 * @param[in] childProcess ID of the child process.
 */
void enableNormalInput(const pid_t childProcess) {
    kill(childProcess,SIGUSR2);
}
