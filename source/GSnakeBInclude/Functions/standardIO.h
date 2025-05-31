/**
 * @file standardIO.h
 * @brief This header declares all the functions about standard I/O.
 */

#ifndef STANDARD_IO_H
#define STANDARD_IO_H

#include "GSnakeBInclude/Struct/GameAllRunningData.h"

#include <unistd.h>

/**
 * @brief Wait for user input with timeout using poll system call.
 * 
 * @param initialTimeoutMinutes Initial timeout in minutes (can
 *        be overridden by user input).
 * @return int 0 if user input detected, 1 if timeout reached, 2
 *             if system call poll(2) return in error.
 */
int waitForUserInputWithPoll(double initialTimeoutMinutes);

/**
 * @brief Block waiting for user input.
 */
void blockWaitUserEnter();

/**
 * @brief Non-blocking keyboard check
 * @return More than 0 if key pressed, less than or equal to 0
 *         otherwise.
 */
int linuxKbhit();

#endif
