/**
 * @file standardIO.c
 * @brief This source realizes the functions about standard I/O.
 */

#include "include/Functions/standardIO.h"
#include <poll.h>
#include <unistd.h>

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


