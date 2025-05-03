/**
 * @file standardIO.h
 * @brief This header declares all the functions about standard I/O.
 */

#ifndef STANDARD_IO_H
#define STANDARD_IO_H

#include "../Compat/snakeFullCompat.h"

/**
 * @brief Block waiting for user input.
 */
void blockWaitUserEnter();

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
void disableNormalInput(const pid_t childProcess);

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
void enableNormalInput(const pid_t childProcess);

#endif
