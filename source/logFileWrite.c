#include "GSnakeBInclude/LogFile/logFileWrite.h"

#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

// Platform-specific includes
#if defined (_WIN32) | defined (_WIN64)   // Windows

#include <io.h>
#include <windows.h>

// Windows-specific definitions
#define stat                               _stat

// Map pthread functions to Windows equivalents
#define pthread_mutex_init(log_mutex, ...) InitializeCriticalSection((log_mutex))
#define pthread_mutex_destroy(log_mutex)   DeleteCriticalSection((log_mutex))
#define pthread_mutex_lock(log_mutex)      EnterCriticalSection((log_mutex))
#define pthread_mutex_unlock(log_mutex)    LeaveCriticalSection((log_mutex))

CRITICAL_SECTION log_mutex;  // Windows mutex for log operations

#else   // Linux, MacOS, Android, etc.

#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

pthread_mutex_t log_mutex;  // POSIX mutex for log operations

#endif // defined (_WIN32) | defined (_WIN64)

// Configuration constants
#define LOG_BATCH_SIZE      32     ///< Number of log entries per batch
#define LOG_BUFFER_TIMEOUT  2      ///< Max buffer hold time in seconds
#define MAX_SINGLE_LOG_LEN  1024   ///< Max length of single log message
#define MAX_LOG_FILES       16     ///< Max number of log files supported

/**
 * Structure for buffered log entries
 * Holds log data before writing to file
 */
typedef struct {
    LogLevel level;               ///< Log severity level
    char message[MAX_SINGLE_LOG_LEN];  ///< Formatted log message
    time_t timestamp;             ///< Time when log was created
    char filename[PATH_MAX];      ///< Target log file path
} LogEntry;

// Static variables for batch writing
static LogEntry log_buffer[LOG_BATCH_SIZE];  ///< Circular buffer for log entries
static int buffer_count = 0;                 ///< Current number of buffered entries
static time_t last_flush_time = 0;           ///< Last buffer flush timestamp
static pthread_mutex_t buffer_mutex;         ///< Mutex for buffer operations

static unsigned long long int MAX_LOG_SIZE = 64*1024*1024;  ///< Default max log size (64MB)

/// String representations of log levels
static const char * level_strings[] = {
    "DEBUG",     ///< LOG_DEBUG
    "INFO",      ///< LOG_INFO
    "WARNING",   ///< LOG_WARNING
    "ERROR",     ///< LOG_ERROR
    "CRITICAL",  ///< LOG_CRITICAL
    NULL         ///< LOG_NOTIP (no prefix)
};

/**
 * File lock structure
 * Tracks open file handles/descriptors for each log file
 */
typedef struct {
#if defined (_WIN32) || defined (_WIN64)
    HANDLE hFile;           ///< Windows file handle
#else
    int fd;                 ///< POSIX file descriptor
#endif
    char filename[PATH_MAX];  ///< Associated log file path
} FileLock;

static FileLock file_locks[MAX_LOG_FILES];  ///< Pool of file locks
static int file_lock_count = 0;             ///< Count of active file locks
static volatile int log_system_initialized = 0;  ///< System initialization flag
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;  ///< Init mutex

/**
 * Automatic initialization function
 * Called before main() executes (constructor attribute)
 */
__attribute__((constructor)) static void auto_init_log_system() {
    if (!log_system_initialized) {
        pthread_mutex_lock(&init_mutex);
        if (!log_system_initialized) {
#if !defined(_WIN32) && !defined(_WIN64)
            pthread_mutex_init(&log_mutex, NULL);
#endif
            pthread_mutex_init(&buffer_mutex, NULL);
            log_system_initialized = 1;
        }
        pthread_mutex_unlock(&init_mutex);
    }
}

/**
 * Automatic cleanup function
 * Called after main() exits (destructor attribute)
 */
__attribute__((destructor)) static void auto_cleanup_log_system() {
    if (log_system_initialized) {
        flushBuffer();  // Flush any remaining logs
        
        pthread_mutex_lock(&init_mutex);
        if (log_system_initialized) {
#if !defined(_WIN32) && !defined(_WIN64)
            pthread_mutex_destroy(&log_mutex);
#endif
            pthread_mutex_destroy(&buffer_mutex);
            
            // Close all open file handles
            for (int i = 0; i < file_lock_count; i++) {
#if defined (_WIN32) || defined (_WIN64)
                if (file_locks[i].hFile != INVALID_HANDLE_VALUE) {
                    CloseHandle(file_locks[i].hFile);
                }
#else
                if (file_locks[i].fd > 0) {
                    close(file_locks[i].fd);
                }
#endif
            }
            log_system_initialized = 0;
        }
        pthread_mutex_unlock(&init_mutex);
    }
}

/**
 * Get existing or create new file lock
 * @param filename Log file path
 * @return Pointer to FileLock, or NULL if no slots available
 */
static FileLock* getFileLock(const char* filename) {
    // Check for existing lock
    for (int i = 0; i < file_lock_count; i++) {
        if (strcmp(file_locks[i].filename, filename) == 0) {
            return &file_locks[i];
        }
    }

    // Create new lock if space available
    if (file_lock_count < MAX_LOG_FILES) {
        strncpy(file_locks[file_lock_count].filename, filename, PATH_MAX);
        file_locks[file_lock_count].filename[PATH_MAX-1] = '\0';
        return &file_locks[file_lock_count++];
    }

    return NULL;
}

/**
 * Initialize file lock structure
 * @param lock FileLock to initialize
 * @param filename Log file path
 * @return 0 on success, -1 on error
 */
static int initFileLock(FileLock * lock, const char * filename) {
#if defined (_WIN32) || defined (_WIN64)
    lock->hFile = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    return (lock->hFile == INVALID_HANDLE_VALUE) ? -1 : 0;
#else
    lock->fd = open(filename, O_WRONLY|O_CREAT|O_APPEND, 0644);
    return (lock->fd == -1) ? -1 : 0;
#endif
}

/**
 * Lock log file for exclusive access
 * @param lock Initialized FileLock
 * @return 0 on success, -1 on error
 */
static int lockFile(FileLock * lock) {
#if defined (_WIN32) || defined (_WIN64)
    OVERLAPPED ov = {0};
    return LockFileEx(lock->hFile, LOCKFILE_EXCLUSIVE_LOCK,
                    0, MAXDWORD, MAXDWORD, &ov) ? 0 : -1;
#else
    struct flock fl = {
        .l_type = F_WRLCK,    // Write lock
        .l_whence = SEEK_SET, // From start of file
        .l_start = 0,         // Entire file
        .l_len = 0            // Until EOF
    };
    return fcntl(lock->fd, F_SETLKW, &fl);  // Blocking lock
#endif
}

/**
 * Unlock previously locked file
 * @param lock Initialized FileLock
 * @return 0 on success, -1 on error
 */
static int unlockFile(FileLock * lock) {
#if defined (_WIN32) || defined (_WIN64)
    OVERLAPPED ov = {0};
    return UnlockFileEx(lock->hFile, 0, MAXDWORD, MAXDWORD, &ov) ? 0 : -1;
#else
    struct flock fl = {
        .l_type = F_UNLCK,    // Unlock
        .l_whence = SEEK_SET, // From start of file
        .l_start = 0,         // Entire file
        .l_len = 0            // Until EOF
    };
    return fcntl(lock->fd, F_SETLK, &fl);  // Non-blocking unlock
#endif
}

//< Explicit function is used
static void closeFileLock(FileLock * lock) __attribute__((used));

/**
 * Close file lock and release resources
 * @param lock FileLock to close
 */
static void closeFileLock(FileLock * lock) {
#if defined (_WIN32) || defined (_WIN64)
    if (lock->hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(lock->hFile);
        lock->hFile = INVALID_HANDLE_VALUE;
    }
#else
    if (lock->fd > 0) {
        close(lock->fd);
        lock->fd = 0;
    }
#endif
}

/**
 * Flush buffered logs for specific file
 * @param filename Target log file path
 * @return 0 on success, error code on failure
 */
static int flush_logs_to_file(const char *filename) {
    FileLock* lock = getFileLock(filename);
    if (!lock) return ENOMEM;

    FILE *logfile = NULL;
    char time_str[100];
    int ret = 0;

    // Initialize file lock if needed
    if (lock->filename[0] != '\0' && 
#if defined (_WIN32) || defined (_WIN64)
        lock->hFile == INVALID_HANDLE_VALUE
#else
        lock->fd == 0
#endif
    ) {
        if (initFileLock(lock, filename)) {
            return errno;
        }
    }

    // Lock the file
    if (lockFile(lock)) {
        return errno;
    }

    // Open log file
    logfile = fopen(filename, "a");
    if (logfile == NULL) {
        int errnum = errno;
        unlockFile(lock);
        return errnum;
    }

    // Set I/O buffer
    if (setvbuf(logfile, NULL, _IOFBF, 4096)) {
        int errnum = errno;
        fclose(logfile);
        unlockFile(lock);
        return errnum;
    }

    // Write all buffered logs for this file
    for (int i = 0; i < buffer_count; ) {
        if (strcmp(log_buffer[i].filename, filename) == 0) {
            // Handle NOTIP level (no formatting)
            if (log_buffer[i].level == LOG_NOTIP) {
                fputs(log_buffer[i].message, logfile);
            } else {
                // Format timestamp
                if (strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", 
                            localtime(&log_buffer[i].timestamp)) == 0) {
                    ret = errno;
                    break;
                }
                // Write formatted log
                fprintf(logfile, "[%s] [%s] %s\n", 
                        time_str, 
                        level_strings[log_buffer[i].level], 
                        log_buffer[i].message);
            }

            // Remove entry from buffer
            if (i < buffer_count - 1) {
                memmove(&log_buffer[i], &log_buffer[i+1], 
                       (buffer_count - i - 1) * sizeof(LogEntry));
            }
            buffer_count--;
        } else {
            i++;
        }
    }

    // Ensure data is written
    if (fflush(logfile) == EOF) ret = errno;
    if (fclose(logfile) == EOF && ret == 0) ret = errno;
    if (unlockFile(lock) && ret == 0) ret = errno;

    return ret;
}

int setRotation(unsigned int size) {
    if (size > 1024*1024 || size < 1) {
        return EINVAL;
    }
    MAX_LOG_SIZE = size*1024*1024;
    return 0;
}

int rotateLog(const char * filename) {
    if (strlen(filename) >= PATH_MAX) {
        return ENAMETOOLONG;
    }

    struct stat st;
    char oldname[PATH_MAX], newname[PATH_MAX];
    time_t now;
    char time_str[100];
    
    if (stat(filename, &st) == -1) return errno;

    if (st.st_size > MAX_LOG_SIZE) {
        if (time(&now) == (time_t)-1) return errno;
        if (strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", localtime(&now)) == 0) return errno;
        if (snprintf(oldname, sizeof(oldname), "%s", filename) >= (int)sizeof(oldname)) return ENAMETOOLONG;
        if (snprintf(newname, sizeof(newname), "%s.%s.bak", filename, time_str) >= (int)sizeof(newname)) return ENAMETOOLONG;
        if (rename(oldname, newname) == -1) return errno;
        return 1;
    }
    return 0;
}

int flushBuffer() {
    if (!log_system_initialized) return 0;
    
    pthread_mutex_lock(&buffer_mutex);
    
    int ret = 0;
    char unique_files[MAX_LOG_FILES][PATH_MAX];
    int unique_count = 0;
    
    // Collect unique filenames
    for (int i = 0; i < buffer_count && unique_count < MAX_LOG_FILES; i++) {
        bool found = false;
        for (int j = 0; j < unique_count; j++) {
            if (strcmp(unique_files[j], log_buffer[i].filename) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            strncpy(unique_files[unique_count], log_buffer[i].filename, PATH_MAX);
            unique_files[unique_count][PATH_MAX-1] = '\0';
            unique_count++;
        }
    }

    // Flush each file's logs
    for (int i = 0; i < unique_count; i++) {
        int flush_ret = flush_logs_to_file(unique_files[i]);
        if (flush_ret != 0 && ret == 0) {
            ret = flush_ret;
        }
    }

    if (ret == 0) {
        time(&last_flush_time);
    }

    pthread_mutex_unlock(&buffer_mutex);
    return ret;
}

int logMessage(LogLevel level, const char * filename, const char * format, ...) {
    if (!log_system_initialized) return EAGAIN;

    va_list args;
    char message[MAX_SINGLE_LOG_LEN];
    time_t now;
    int ret = 0;

    // Get current time
    if (time(&now) == (time_t)-1) {
        return errno;
    }

    // Format message
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    // Add to buffer
    pthread_mutex_lock(&buffer_mutex);
    if (buffer_count < LOG_BATCH_SIZE) {
        log_buffer[buffer_count].level = level;
        strncpy(log_buffer[buffer_count].message, message, sizeof(log_buffer[buffer_count].message));
        log_buffer[buffer_count].timestamp = now;
        strncpy(log_buffer[buffer_count].filename, filename, PATH_MAX);
        log_buffer[buffer_count].filename[PATH_MAX-1] = '\0';
        buffer_count++;
    }

    // Check if flush needed
    bool need_flush = (buffer_count >= LOG_BATCH_SIZE) || 
                     (buffer_count > 0 && (now - last_flush_time) >= LOG_BUFFER_TIMEOUT);
    pthread_mutex_unlock(&buffer_mutex);

    if (need_flush) {
        ret = flushBuffer();
    }

    // Fallback to direct write if buffer full
    if (ret != 0 && buffer_count >= LOG_BATCH_SIZE) {
        FileLock* lock = getFileLock(filename);
        if (!lock) return ENOMEM;

        pthread_mutex_lock(&log_mutex);

        if (lock->filename[0] != '\0' && 
#if defined (_WIN32) || defined (_WIN64)
            lock->hFile == INVALID_HANDLE_VALUE
#else
            lock->fd == 0
#endif
        ) {
            if (initFileLock(lock, filename)) {
                pthread_mutex_unlock(&log_mutex);
                return errno;
            }
        }

        if (lockFile(lock)) {
            pthread_mutex_unlock(&log_mutex);
            return errno;
        }

        int rotate_ret = rotateLog(filename);
        if (rotate_ret != 0 && rotate_ret != 1) {
            unlockFile(lock);
            pthread_mutex_unlock(&log_mutex);
            return rotate_ret == -1 ? errno : rotate_ret;
        }

        FILE *logfile = fopen(filename, "a");
        if (logfile == NULL) {
            int errnum = errno;
            unlockFile(lock);
            pthread_mutex_unlock(&log_mutex);
            return errnum;
        }

        if (setvbuf(logfile, NULL, _IOFBF, 4096)) {
            int errnum = errno;
            fclose(logfile);
            unlockFile(lock);
            pthread_mutex_unlock(&log_mutex);
            return errnum;
        }

        if (level == LOG_NOTIP) {
            fputs(message, logfile);
        } else {
            char time_str[100];
            if (strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now)) == 0) {
                fclose(logfile);
                unlockFile(lock);
                pthread_mutex_unlock(&log_mutex);
                return errno;
            }
            fprintf(logfile, "[%s] [%s] %s\n", time_str, level_strings[level], message);
        }

        if (fflush(logfile) == EOF) ret = errno;
        if (fclose(logfile) == EOF && ret == 0) ret = errno;
        if (unlockFile(lock) && ret == 0) ret = errno;

        pthread_mutex_unlock(&log_mutex);
    }

    return ret;
}
