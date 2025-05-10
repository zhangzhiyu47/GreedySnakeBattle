/**
 * @file standardIO.h
 * @brief This header declares all the functions about standard I/O.
 */

#ifndef STANDARD_IO_H
#define STANDARD_IO_H

#include <unistd.h>

/**
 * @brief Block waiting for user input.
 */
void blockWaitUserEnter();

/**
 * @brief Enable normal input mode (echo + line buffering)
 */
void trulyEnableNormalInput();

/**
 * @brief Disable normal input mode (no echo + no line buffering)
 * @ingroup OSAdapt
 */
void trulyDisableNormalInput();

/**
 * @brief Non-blocking keyboard check
 * @return More than 0 if key pressed, less than or equal to 0
 *         otherwise.
 */
int linuxKbhit();

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
void disableNormalInput(const pid_t childProcess);

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
void enableNormalInput(const pid_t childProcess);

#endif
