/**
 * @file snakeFullCompat.c
 * @brief Implementation of cross-platform compatibility functions
 * 
 * This file contains all the platform-specific implementations for
 * terminal control, process management, and input handling.
 */

#include "Include/Compat/snakeFullCompat.h"

/**
 * @name Windows-specific Implementations
 */
/** @{ */
#if SNAKE_OS_WINDOWS

static DWORD _original_console_mode; /**< Original Windows console mode */

/**
 * @brief Initialize Windows console settings
 * @ingroup OSAdapt
 * 
 * Saves original console mode and enables ANSI escape code support.
 */
static void _windows_init_console() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &_original_console_mode);
    
    // Enable ANSI escape codes on Windows 10+
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hStdout, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, dwMode);
}

/**
 * @brief Set Windows console input mode
 * @ingroup OSAdapt
 * @param echo Enable/disable echo of input characters
 * @param line_input Enable/disable line input mode
 */
static void _windows_set_input_mode(int echo, int line_input) {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = _original_console_mode;
    
    if (echo) mode |= ENABLE_ECHO_INPUT;
    else mode &= ~ENABLE_ECHO_INPUT;
    
    if (line_input) mode |= ENABLE_LINE_INPUT;
    else mode &= ~ENABLE_LINE_INPUT;
    
    SetConsoleMode(hStdin, mode);
}

/**
 * @brief Windows fork() implementation using CreateProcess
 * @ingroup OSAdapt
 * @return Process ID of child process, or -1 on error
 */
pid_t _snake_fork(void) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    char cmdline[MAX_PATH];
    
    GetModuleFileName(NULL, cmdline, MAX_PATH);
    if (CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 
                    CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        return (pid_t)pi.dwProcessId;
    }
    return (pid_t)-1;
}

/**
 * @brief Windows waitpid() implementation
 * @ingroup OSAdapt
 * @param pid Process ID to wait for
 * @param status Pointer to store exit status
 * @param options Wait options (unused in Windows)
 * @return Process ID if successful, -1 on error
 */
pid_t _snake_waitpid(pid_t pid, int *status, int options) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);
    if (hProcess) {
        WaitForSingleObject(hProcess, INFINITE);
        GetExitCodeProcess(hProcess, (DWORD*)status);
        CloseHandle(hProcess);
        return pid;
    }
    return (pid_t)-1;
}

/**
 * @brief Windows prctl() implementation (limited functionality)
 * @ingroup OSAdapt
 * @param option Control option (only PR_SET_PDEATHSIG supported)
 * @param ... Additional arguments
 * @return 0 on success, -1 on error
 */
int _snake_prctl(int option, ...) {
    if (option == PR_SET_PDEATHSIG) {
        HANDLE hJob = CreateJobObject(NULL, NULL);
        if (hJob) {
            AssignProcessToJobObject(hJob, GetCurrentProcess());
            CloseHandle(hJob);
            return 0;
        }
    }
    return -1;
}

/**
 * @brief Windows sigaction() implementation
 * @ingroup OSAdapt
 * @param sig Signal number (only SIGINT supported)
 * @param act Pointer to action structure
 * @param oldact Pointer to old action structure (unused)
 * @return 0 on success, -1 on error
 */
int _snake_sigaction(int sig, const struct sigaction *act, 
                    struct sigaction *oldact) {
    if (sig == SIGINT) {
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)act->sa_handler, TRUE);
        return 0;
    }
    return -1;
}

/**
 * @brief Windows ioctl() implementation (TIOCGWINSZ only)
 * @ingroup OSAdapt
 * @param fd File descriptor (unused)
 * @param cmd Command (must be TIOCGWINSZ)
 * @param ws Pointer to winsize structure
 * @return 0 on success, -1 on error
 */
int _snake_ioctl(int fd, int cmd, struct winsize *ws) {
    if (cmd == TIOCGWINSZ) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        ws->ws_row = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        ws->ws_col = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        return 0;
    }
    return -1;
}

/**
 * @brief Enable Windows VT (Virtual Terminal) mode
 * @ingroup OSAdapt
 */
static void _enable_vt_mode(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

/**
 * @brief Hide cursor in Windows console
 * @ingroup OSAdapt
 */
static void _windows_hide_cursor(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = 100;
    cci.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cci);
}

/**
 * @brief Show cursor in Windows console
 * @ingroup OSAdapt
 */
static void _windows_show_cursor(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = 100;
    cci.bVisible = TRUE;
    SetConsoleCursorInfo(hOut, &cci);
}
/** @} */

/**
 * @name Unix-specific Implementations
 */
/** @{ */
#else

static struct termios _oldt, _newt; /**< Terminal settings storage */

/**
 * @brief Non-blocking keyboard check for Unix
 * @ingroup OSAdapt
 * @return 1 if key pressed, 0 otherwise
 */
int _snake_kbhit(void) {
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO+1, &fds, NULL, NULL, &tv) > 0;
}

/**
 * @brief Get character from keyboard for Unix
 * @ingroup OSAdapt
 * @return The pressed character
 */
int _snake_getch(void) {
    return getchar();
}

#endif
/** @} */

/**
 * @name Common Terminal Functions
 */
/** @{ */
#if !SNAKE_OS_WINDOWS
static struct termios _original_termios; /**< Original terminal settings */
#endif

/**
 * @brief Initialize terminal settings
 * @ingroup OSAdapt
 */
void init_terminal_settings() {
#if SNAKE_OS_WINDOWS
    _windows_init_console();
#else
    tcgetattr(STDIN_FILENO, &_original_termios);
#endif
}

/**
 * @brief Restore original terminal settings
 * @ingroup OSAdapt
 */
void restore_terminal_settings() {
#if SNAKE_OS_WINDOWS
    // Windows automatically restores settings
#else
    tcsetattr(STDIN_FILENO, TCSANOW, &_original_termios);
#endif
}

/**
 * @brief Enable normal input mode (echo + line buffering)
 * @ingroup OSAdapt
 */
void enable_normal_input() {
#if SNAKE_OS_WINDOWS
    _windows_set_input_mode(1, 1);
#else
    struct termios newt = _original_termios;
    newt.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif
}

/**
 * @brief Disable normal input mode (no echo + no line buffering)
 * @ingroup OSAdapt
 */
void disable_normal_input() {
#if SNAKE_OS_WINDOWS
    _windows_set_input_mode(0, 0);
#else
    struct termios newt = _original_termios;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif
}

/**
 * @brief Get buffered input (for menus)
 * @ingroup OSAdapt
 * @return The input character
 */
int get_buffered_input() {
    enable_normal_input();
    int c = getchar();
    disable_normal_input();
    return c;
}

/**
 * @brief Get unbuffered input (for game control)
 * @ingroup OSAdapt
 * @return The input character
 */
int get_unbuffered_input() {
    disable_normal_input();
#if SNAKE_OS_WINDOWS
    return _getch();
#else
    return getchar();
#endif
}
/** @} */

/**
 * @name Automatic Initialization
 */
/** @{ */
#if SNAKE_OS_WINDOWS
/**
 * @brief Constructor function for Windows initialization
 * @ingroup OSAdapt
 */
static void __attribute__((constructor)) _snake_init(void) {
    _enable_vt_mode();
    _windows_hide_cursor();
}
#else
/**
 * @brief Constructor function for Unix initialization
 * @ingroup OSAdapt
 */
static void __attribute__((constructor)) _snake_init(void) {
    tcgetattr(STDIN_FILENO, &_oldt);
    _newt = _oldt;
    _newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &_newt);
    printf("\033[?25l");
}

/**
 * @brief Destructor function for Unix cleanup
 * @ingroup OSAdapt
 */
static void __attribute__((destructor)) _snake_cleanup(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &_oldt);
    printf("\033[?25h");
}
#endif
/** @} */
