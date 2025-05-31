/**
 * @file signalCapture.h
 * @brief This header declares the functions about signal capturing.
 */
#ifndef SIGNAL_CAPTURE_H
#define SIGNAL_CAPTURE_H

/**
 * @defgroup SignalCapture Signal capture
 * @brief These functions are used for signal capture.
 *
 * These functions are all used for signals. The naming
 * convention for these functions is:  
 * `[father/child]ProcessCatch[signal name (INT/CHLD/USR1/USR2/...)].`  
 * For example, the function for the parent process to capture
 * the **SIGINT** signal is @ref fatherProcessCatchINT.
 */

/**
 * @brief Father process captures signal **SIGINT**.
 * @ingroup SignalCapture
 *
 * Exit the game application after the signal **SIGINT** is
 * delivered (only exit the parent process).
 *
 * @param[in] signum The number of the captured signal.
 */
void fatherProcessCatchINT(int signum);

/**
 * @brief Father process captures signal **SIGCHLD**.
 * @ingroup SignalCapture
 *
 * Capture the signal **SIGCHLD** and try to recycle the
 * subprocess to judge whether the subprocess is dead.
 * If dead, exit the game application (that is, the parent
 * process).
 *
 * @param[in] signum The number of the captured signal.
 */
void fatherProcessCatchCHLD(int signum);

/**
 * @brief Child process captures signal **SIGINT**.
 * @ingroup SignalCapture
 *
 * Exit this child process after the signal **SIGINT** is
 * delivered.
 *
 * @param[in] signum The number of the captured signal.
 */
void childProcessCatchINT(int signum);

#endif
