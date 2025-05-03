/**
 * @file snakeFullCompat.h
 * @brief Cross-platform compatibility layer for Snake game
 * 
 * Provides unified interface for terminal control, process management,
 * and input handling across Windows and Unix-like systems.
 */

#ifndef SNAKE_FULL_COMPAT_H
#define SNAKE_FULL_COMPAT_H

#include <stdio.h>
#include <stdlib.h>

/**
 * @defgroup OSAdapt Operating system adaptation function group
 * @brief These functions are used to adapt different
 *        operating systems.
 *
 * These adaptation interface functions usually start with' _', and words
 * are separated by' _', which also provides interface data types
 * and variables for WindowsOS. Used to adapt to Windows and UNIX
 * -Like(such as Linux,MacOs,Termux and Android) operating systems.
 * You don't need to call these functions by yourself. These
 * functions are called in other codes.
 *
 * @warning Try not to call these functions by yourself, because
 *          these function definitions are complicated and many
 *          macro definitions are nested, unless it is necessary
 *          or you have enough code reading ability.
 *
 * @note I hope this document can help you a lot.
 */

// ==================== OS Detection ====================
#if defined(_WIN32) || defined(_WIN64)
    #define SNAKE_OS_WINDOWS 1
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <process.h>
    #include <conio.h>
#else
    #define SNAKE_OS_WINDOWS 0
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/ioctl.h>
    #include <sys/prctl.h>
    #include <termios.h>
    #include <signal.h>
#endif

// ==================== Type Compatibility ====================
#if SNAKE_OS_WINDOWS
    /** Process ID type for Windows compatibility */
    typedef int pid_t;
    
    /** Microsecond type for Windows compatibility */
    typedef unsigned int useconds_t;
    
    /** Terminal window size structure for Windows */
    struct winsize {
        unsigned short ws_row; /**< Number of rows */
        unsigned short ws_col; /**< Number of columns */
    };
    
    /** Simplified sigaction for Windows */
    struct sigaction {
        void (*sa_handler)(int); /**< Signal handler function */
    };
    
    // Standard file descriptors for Windows
    #define STDIN_FILENO 0
    #define STDOUT_FILENO 1
    #define STDERR_FILENO 2
#endif

// ==================== Process Control ====================
#if SNAKE_OS_WINDOWS
    /**
     * @brief Windows fork() implementation using CreateProcess
     * @ingroup OSAdapt
     * @return Process ID of child process, or -1 on error
     */
    #define fork() _snake_fork()
    pid_t _snake_fork(void);
    
    /**
     * @brief Windows waitpid() implementation
     * @ingroup OSAdapt
     * @param pid Process ID to wait for
     * @param status Pointer to store exit status
     * @param options Wait options (unused in Windows)
     * @return Process ID if successful, -1 on error
     */
    #define waitpid(pid, status, options) _snake_waitpid(pid, status, options)
    pid_t _snake_waitpid(pid_t pid, int *status, int options);
    
    /**
     * @brief Windows prctl() implementation (limited functionality)
     * @ingroup OSAdapt
     * @param option Control option (only PR_SET_PDEATHSIG supported)
     * @param ... Additional arguments
     * @return 0 on success, -1 on error
     */
    #define prctl(option, ...) _snake_prctl(option, ##__VA_ARGS__)
    int _snake_prctl(int option, ...);
    
    /**
     * @brief Windows sigaction() implementation
     * @ingroup OSAdapt
     * @param sig Signal number (only SIGINT supported)
     * @param act Pointer to action structure
     * @param oldact Pointer to old action structure (unused)
     * @return 0 on success, -1 on error
     */
    #define sigaction(sig, act, oldact) _snake_sigaction(sig, act, oldact)
    int _snake_sigaction(int sig, const struct sigaction *act, 
                        struct sigaction *oldact);
#endif

// ==================== Time Functions ====================
#if SNAKE_OS_WINDOWS
    /** Microsecond sleep for Windows */
    #define usleep(us) Sleep((DWORD)((us)/1000))
    /** Second sleep for Windows */
    #define sleep(s) Sleep((DWORD)((s)*1000))
#endif

// ==================== Terminal Control ====================
#if SNAKE_OS_WINDOWS
    /**
     * @brief Windows ioctl() implementation (TIOCGWINSZ only)
     * @ingroup OSAdapt
     * @param fd File descriptor (unused)
     * @param cmd Command (must be TIOCGWINSZ)
     * @param ws Pointer to winsize structure
     * @return 0 on success, -1 on error
     */
    #define ioctl(fd, cmd, arg) _snake_ioctl(fd, cmd, arg)
    int _snake_ioctl(int fd, int cmd, struct winsize *ws);
    
    // Keyboard input functions
    #define linux_kbhit() _kbhit()
    #define linux_getch() _getch()
#else
    /**
     * @brief Non-blocking keyboard check for Unix
     * @ingroup OSAdapt
     * @return 1 if key pressed, 0 otherwise
     */
    #define linux_kbhit() _snake_kbhit()
    int _snake_kbhit(void);
    
    /**
     * @brief Get character from keyboard for Unix
     * @ingroup OSAdapt
     * @return The pressed character
     */
    #define linux_getch() _snake_getch()
    int _snake_getch(void);
#endif

// ==================== Terminal Management ====================
/**
 * @brief Initialize terminal settings
 * @ingroup OSAdapt
 * 
 * Should be called at program startup to save original terminal state.
 */
void init_terminal_settings();

/**
 * @brief Restore original terminal settings
 * @ingroup OSAdapt
 * 
 * Should be called at program exit to restore terminal to original state.
 */
void restore_terminal_settings();

/**
 * @brief Enable normal input mode (echo + line buffering)
 * @ingroup OSAdapt
 */
void enable_normal_input();

/**
 * @brief Disable normal input mode (no echo + no line buffering)
 * @ingroup OSAdapt
 */
void disable_normal_input();

/**
 * @brief Get buffered input (for menus)
 * @ingroup OSAdapt
 * @return The input character
 */
int get_buffered_input();

/**
 * @brief Get unbuffered input (for game control)
 * @ingroup OSAdapt
 * @return The input character
 */
int get_unbuffered_input();

// ==================== Unified Control Macros ====================
/** Clear terminal screen */
#define clearAll() (SNAKE_OS_WINDOWS ? system("cls") : system("clear"))

/** Hide terminal cursor */
#define HIDE_CURSOR() (SNAKE_OS_WINDOWS ? _windows_hide_cursor() : printf("\033[?25l"))

/** Show terminal cursor */
#define SHOW_CURSOR() (SNAKE_OS_WINDOWS ? _windows_show_cursor() : printf("\033[?25h"))

/** Set cursor position */
#define SET_CURSOR_POS(y,x) (SNAKE_OS_WINDOWS ? \
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), (COORD){(SHORT)(x), (SHORT)(y)}) : \
    printf("\033[%d;%dH", (y), (x)))

// ==================== Color Support ====================
/** Set terminal color using ANSI escape codes */
#define printf_color(fg, bg) \
    do { \
        if (SNAKE_OS_WINDOWS) { \
            printf("\033[38;5;%d;48;5;%dm", (fg), (bg)); \
        } else { \
            printf("\033[38;5;%d;48;5;%dm", (fg), (bg)); \
        } \
    } while(0)

#endif // SNAKE_FULL_COMPAT_H
