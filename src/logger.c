#include "include/logger.h"

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <stdatomic.h>


// Platform-specific includes
#if defined (_WIN32) | defined (_WIN64)   // Windows


#include <io.h>
#include <windows.h>

// Windows-specific definitions
#define stat                               _stat

typedef SRWLOCK pthread_mutex_t;
#define PTHREAD_MUTEX_INITIALIZER SRWLOCK_INIT
#define XFD_INVALID INVALID_HANDLE_VALUE

static inline int pthread_mutex_init(pthread_mutex_t *mutex, void *attr) {
    (void)attr;
    InitializeSRWLock(mutex);
    return 0;
}

static inline int pthread_mutex_destroy(pthread_mutex_t *mutex) {
    (void)mutex;
    return 0;
}

static inline int pthread_mutex_lock(pthread_mutex_t *mutex) {
    AcquireSRWLockExclusive(mutex);
    return 0;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    ReleaseSRWLockExclusive(mutex);
    return 0;
}

static inline int pthread_mutex_trylock(pthread_mutex_t *mutex) {
    return TryAcquireSRWLockExclusive(mutex) ? 0 : EBUSY;
}


#else   // Linux, MacOS, Android, etc.


#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define XFD_INVALID ((int)-1)


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
    LogLevel level;                    ///< Log severity level
    char message[MAX_SINGLE_LOG_LEN];  ///< Formatted log message
    time_t timestamp;                  ///< Time when log was created
    char filename[PATH_MAX];           ///< Target log file path
} LogEntry;


// Static variables for batch writing
static LogEntry log_buffer[LOG_BATCH_SIZE];  ///< Circular buffer for log entries
static int buffer_count = 0;                 ///< Current number of buffered entries
static time_t last_flush_time = 0;           ///< Last buffer flush timestamp

static pthread_mutex_t log_mutex;            ///< Mutex for log operations
static pthread_mutex_t log_xfd_mutex;        ///< Mutex for log xfd operations
static pthread_mutex_t buffer_mutex;         ///< Mutex for buffer operations
static pthread_mutex_t default_log_mutex;    ///< Mutex for default log operations
static pthread_mutex_t default_xfd_mutex;    ///< Mutex for default xfd operations

static int64_t MAX_LOG_SIZE = ROT_DEFAULT;   ///< Default max log size
static char default_log_file[PATH_MAX] = {}; ///< Global default log file
static xfd_t default_xfd;                    ///< Global default xfd
static bool default_color;                   ///< Global default color


/// String representations of log levels
static const char * level_strings[] = {
    "DEBUG",     ///< LOG_DEBUG
    "INFO",      ///< LOG_INFO
    "WARNING",   ///< LOG_WARNING
    "ERROR",     ///< LOG_ERROR
    "CRITICAL",  ///< LOG_CRITICAL
    NULL,        ///< LOG_NOTIP (no prefix)
};

#define ANSI_RESET   "\033[0m"
#define ANSI_RC      "\033[7m"
#define ANSI_GRAY    "\033[3;90m"
#define ANSI_WHITE   "\033[37m"
#define ANSI_BULE    "\033[94m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_RED     "\033[31m"
#define ANSI_BRED    "\033[1;91m"

static const char *level_colors[] = {
    ANSI_WHITE,   ///< LOG_DEBUG
    ANSI_BULE,    ///< LOG_INFO
    ANSI_YELLOW,  ///< LOG_WARNING
    ANSI_RED,     ///< LOG_ERROR
    ANSI_BRED,    ///< LOG_CRITICAL
};

/**
 * File lock structure
 * Tracks open file handles/descriptors for each log file
 */
typedef struct {
#if defined (_WIN32) || defined (_WIN64)
    HANDLE hFile;             ///< Windows file handle
#else
    int fd;                   ///< POSIX file descriptor
#endif
    char filename[PATH_MAX];  ///< Associated log file path
} FileLock;

static FileLock file_locks[MAX_LOG_FILES];  ///< Pool of file locks
static int file_lock_count = 0;             ///< Count of active file locks


static atomic_int log_system_initialized = 0;  ///< System initialization flag

void logInit() {
    int expected = 0;

    if (atomic_compare_exchange_strong(&log_system_initialized,
                &expected, 1)) {
        pthread_mutex_init(&log_mutex, NULL);
        pthread_mutex_init(&log_xfd_mutex, NULL);
        pthread_mutex_init(&buffer_mutex, NULL);
        pthread_mutex_init(&default_log_mutex, NULL);
        pthread_mutex_init(&default_xfd_mutex, NULL);

        memset(default_log_file, 0, sizeof(default_log_file));
        default_xfd = XFD_INVALID;
        default_color = false;
        
        buffer_count = 0;
        last_flush_time = 0;
        file_lock_count = 0;
        memset(log_buffer, 0, sizeof(log_buffer));
        memset(file_locks, 0, sizeof(file_locks));
    }
}

void logCleanup() {
    int expected = 1;

    if (atomic_compare_exchange_strong(&log_system_initialized,
                &expected, 0)) {
        flush();  // Flush any remaining logs
        
        pthread_mutex_destroy(&log_mutex);
        pthread_mutex_destroy(&log_xfd_mutex);
        pthread_mutex_destroy(&buffer_mutex);
        pthread_mutex_destroy(&default_log_mutex);
        pthread_mutex_destroy(&default_xfd_mutex);

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

        memset(default_log_file, 0, sizeof(default_log_file));
        default_xfd = XFD_INVALID;
        default_color = false;

        buffer_count = 0;
        last_flush_time = 0;
        file_lock_count = 0;
        memset(log_buffer, 0, sizeof(log_buffer));
        memset(file_locks, 0, sizeof(file_locks));
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
    if (!atomic_load(&log_system_initialized)) return ENOTINIT;

    if (size > 1024*1024 || size < 1) {
        return EINVAL;
    }
    MAX_LOG_SIZE = size*1024*1024;

    return 0;
}

int rotate(const char * filename) {
    if (!atomic_load(&log_system_initialized)) return ENOTINIT;

    if (strlen(filename) >= PATH_MAX) {
        return ENAMETOOLONG;
    }

    struct stat st;
    char oldname[PATH_MAX], newname[PATH_MAX];
    time_t now;
    char time_str[100];
    
    if (stat(filename, &st) == -1) return errno;

    if ((int64_t)st.st_size > MAX_LOG_SIZE) {
        if (time(&now) == (time_t)-1) return errno;
        if (strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", localtime(&now)) == 0) return errno;
        if (snprintf(oldname, sizeof(oldname), "%s", filename) >= (int)sizeof(oldname)) return ENAMETOOLONG;
        if (snprintf(newname, sizeof(newname), "%s.%s.bak", filename, time_str) >= (int)sizeof(newname)) return ENAMETOOLONG;
        if (rename(oldname, newname) == -1) return errno;
        return 1;
    }
    return 0;
}

int flush() {
    if (!atomic_load(&log_system_initialized)) return ENOTINIT;
    
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

int vlog2file(LogLevel level,
        const char * filename,
        const char * format,
        va_list args) {
    if (!atomic_load(&log_system_initialized)) return ENOTINIT;

    char message[MAX_SINGLE_LOG_LEN];
    time_t now;
    int ret = 0;

    // Get current time
    if (time(&now) == (time_t)-1) {
        return errno;
    }

    // Format message
    vsnprintf(message, sizeof(message), format, args);

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
        ret = flush();
    }

    // Fallback to direct write if buffer full
    if (ret != 0 && buffer_count >= LOG_BATCH_SIZE) {
        FileLock* lock = getFileLock(filename);
        if (!lock) return EIO;

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

        int rotate_ret = rotate(filename);
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

int log2file(LogLevel level, const char *filename, const char *format, ...) {
    va_list args;
    int retval = 0;

    va_start(args, format);
    retval = vlog2file(level, filename, format, args);
    va_end(args);

    return retval;
}

int setDefaultLogFile(const char * filename) {
    if (!atomic_load(&log_system_initialized)) return ENOTINIT;
    if (filename == NULL) return EINVAL;
    if (strlen(filename) + 1 > PATH_MAX) return ENAMETOOLONG;

    pthread_mutex_lock(&default_log_mutex);
    strncpy(default_log_file, filename, PATH_MAX);
    pthread_mutex_unlock(&default_log_mutex);

    return 0;
}

int logger(LogLevel level, const char *format, ...) {
    va_list args;
    int retval = 0;

    if (!atomic_load(&log_system_initialized)) return ENOTINIT;

    pthread_mutex_lock(&default_log_mutex);

    if (default_log_file[0] == '\0') {
        pthread_mutex_unlock(&default_log_mutex);
        return ENOTSETDEFAULT;
    }
    
    va_start(args, format);
    retval = vlog2file(level, default_log_file, format, args);
    va_end(args);

    pthread_mutex_unlock(&default_log_mutex);

    return retval;
}

long xfd_write(xfd_t fd, const char *buf, size_t len) {
#ifdef _WIN32
    DWORD written;
    DWORD total;
    DWORD chunk;

    total = 0;
    while (total < len) {
        chunk = (len - total > 0xFFFFFFFFU) ? 
            0xFFFFFFFFU : (DWORD)(len - total);
        if (!WriteFile(fd, buf + total, chunk, &written, NULL)) {
            errno = EIO;
            return -1;
        }
        total += written;
    }
    return (long)total;
#else
    return (long)write(fd, buf, len);
#endif
}

int xfd_flush(xfd_t fd) {
#ifdef _WIN32
    if (!FlushFileBuffers(fd)) {
        errno = EIO;
        return -1;
    }
    return 0;
#else
    return fsync(fd);
#endif
}

int vlog2xfd(LogLevel level,
        xfd_t xfd,
        bool color,
        const char * format,
        va_list args) {
    if (!atomic_load(&log_system_initialized)) return ENOTINIT;

    char message[MAX_SINGLE_LOG_LEN];
    time_t now;
    int ret = 0;
    size_t len = 0;

    // Get current time
    if (time(&now) == (time_t)-1) {
        return errno;
    }

    // Format message
    len = vsnprintf(message, sizeof(message), format, args);

    // Write directly to the file
    pthread_mutex_lock(&log_xfd_mutex);
    if (level == LOG_NOTIP) {
        ret = xfd_write(xfd, message, len);
        if (ret >= 0) {
            ret = 0;
        } else {
            ret = errno;
        }
    } else {
        char time_str[100];
        if (strftime(time_str, sizeof(time_str),
                    "%Y-%m-%d %H:%M:%S", localtime(&now)) == 0) {
            pthread_mutex_unlock(&log_xfd_mutex);
            return errno;
        }

        char log[MAX_SINGLE_LOG_LEN];
        if (color) {
            len = snprintf(log, MAX_SINGLE_LOG_LEN,
                    "[%s%s%s] [%s%s%s%s] %s%s%s\n",
                    ANSI_GRAY, time_str, ANSI_RESET,
                    ANSI_RC, level_colors[level],
                    level_strings[level], ANSI_RESET,
                    level_colors[level], message, ANSI_RESET);
        } else {
            len = snprintf(log, MAX_SINGLE_LOG_LEN, "[%s] [%s] %s\n",
                    time_str, level_strings[level], message);
        }
        ret = xfd_write(xfd, log, len);
        if (ret > 0) {
            ret = 0;
        } else {
            ret = errno;
        }
    }
    xfd_flush(xfd);
    pthread_mutex_unlock(&log_xfd_mutex);

    return ret;
}

int log2xfd(LogLevel level, xfd_t xfd, bool color, const char *format, ...) {
    va_list args;
    int retval = 0;

    va_start(args, format);
    retval = vlog2xfd(level, xfd, color, format, args);
    va_end(args);

    return retval;
}

int setDefaultXfdAndColor(const xfd_t xfd, bool color) {
    if (!atomic_load(&log_system_initialized)) return ENOTINIT;
    if (xfd == XFD_INVALID) return EINVAL;

    pthread_mutex_lock(&default_xfd_mutex);
    default_xfd = xfd;
    default_color = color;
    pthread_mutex_unlock(&default_xfd_mutex);

    return 0;
}

int logxfd(LogLevel level, const char *format, ...) {
    va_list args;
    int retval = 0;

    if (!atomic_load(&log_system_initialized)) return ENOTINIT;

    pthread_mutex_lock(&default_xfd_mutex);

    if (default_xfd == XFD_INVALID) {
        pthread_mutex_unlock(&default_xfd_mutex);
        return ENOTSETDEFAULT;
    }
    
    va_start(args, format);
    retval = vlog2xfd(level, default_xfd, default_color, format, args);
    va_end(args);

    pthread_mutex_unlock(&default_xfd_mutex);

    return retval;
}

DualReturn log2dual(LogLevel level, const LogDual *dual, const char *format, ...) {
    DualReturn ret = {0, 0};
    bool isError = false;
    va_list args;

    if (!atomic_load(&log_system_initialized)) {
        DualReturn ret = {ENOTINIT, ENOTINIT};
        return ret;
    }

    if (dual->route1 == XFD_INVALID) {
        ret.route1 = EINVAL;
        isError = true;
    }
    if (dual->route2 == NULL) {
        ret.route2 = EINVAL;
        isError = true;
    }

    if (isError) {
        return ret;
    }

    va_start(args, format);
    ret.route1 = vlog2xfd(level, dual->route1, dual->color, format, args);
    ret.route2 = vlog2file(level, dual->route2, format, args);
    va_end(args);

    return ret;
}

DualReturn logdual(LogLevel level, const char *format, ...) {
    DualReturn ret = {0, 0};
    va_list args;

    if (!atomic_load(&log_system_initialized)) {
        DualReturn ret = {ENOTINIT, ENOTINIT};
        return ret;
    }

    pthread_mutex_lock(&default_xfd_mutex);
    if (default_xfd == XFD_INVALID) {
        ret.route1 = ENOTSETDEFAULT;
    } else {
        va_start(args, format);
        ret.route1 = vlog2xfd(level, default_xfd, default_color, format, args);
        va_end(args);
    }
    pthread_mutex_unlock(&default_xfd_mutex);
    
    pthread_mutex_lock(&default_log_mutex);
    if (default_log_file[0] == '\0') {
        ret.route2 = ENOTSETDEFAULT;
    } else {
        va_start(args, format);
        ret.route2 = vlog2file(level, default_log_file, format, args);
        va_end(args);
    }
    pthread_mutex_unlock(&default_log_mutex);

    return ret;
}
