#ifndef LOG_FILE_WRITE
#define LOG_FILE_WRITE

// For C++ apps to correctly call APIs
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#if defined (_WIN32) | defined (_WIN64) // Windows
#include <windows.h>
#define XFD_STDOUT GetStdHandle(STD_OUTPUT_HANDLE)
#define XFD_STDERR GetStdHandle(STD_ERROR_HANDLE)
typedef HANDLE xfd_t;
#else // The other POSIX OS
#define XFD_STDOUT STDOUT_FILENO
#define XFD_STDERR STDERR_FILENO
typedef int xfd_t;
#endif // defined (_WIN32) | defined (_WIN64)

#define __to_string(x) #x
#define toString(x) __to_string(x)


#include <stdbool.h>


/**
 * Log level enumeration
 * Defines different severity levels for log messages
 */
typedef enum {
    LOG_DEBUG,    ///< Debugging messages (lowest severity)
    LOG_INFO,     ///< Informational messages
    LOG_WARNING,  ///< Warning conditions
    LOG_ERROR,    ///< Error conditions
    LOG_CRITICAL, ///< Critical conditions (highest severity)
    LOG_NOTIP,    ///< Special level - no prefix/suffix formatting
} LogLevel;

/**
 * Suggested log rotation sizes
 * Provides predefined size options for log rotation
 */
typedef enum {
    ROT_MINIMUM  = 2,          ///< For test environments (2MB)
    ROT_USERAPP  = 4,          ///< For user applications (4MB)
    ROT_HIGHFRE  = 32,         ///< For high frequency logs (32MB)
    ROT_DEFAULT  = 64,         ///< Default size (64MB)
    ROT_ROUTINE  = 256,        ///< For routine application logs (256MB)
    ROT_MEDIUML  = 1024,       ///< For medium/large services (1GB)
    ROT_BIGDATA  = 64 * 1024,  ///< For big data applications (64GB)
    ROT_MAGNANI  = 512 * 1024, ///< For very large systems (512GB)
} RotationRange;

/**
 * Wrong return value
 */
typedef enum {
    ENOTINIT = 1000,   ///< Not initialized
    ENOTSETDEFAULT,    ///< Not set default log file
} LogError;

/**
 * Dual output logs
 */
typedef struct {
    xfd_t  route1;    ///< Route 1: Use @log2xfd to output to a xfd
    char * route2;    ///< Route 2: Use @log2file to output to a file
    bool   color;     ///< Color option for @log2xfd
} LogDual;

/**
 * Dual return value
 */
typedef struct {
    int route1;
    int route2;
} DualReturn;

/**
 * Initialize the logging system (Only once). You need to call
 * it before any log operations, otherwise you'll get an @ENOTINIT error.
 * @warning Thread unsafe
 */
void logInit(void);

/**
 * Clean up the logging system. Once cleaned, the logging
 * system will no longer be usable
 */
void logCleanup(void);

/**
 * Set maximum log file size before rotation (Global impact)
 * @param size Size in megabytes (1MB to 1PB)
 * @return 0 on success, error code on failure. Actually,
 *         if you've already initialized, it always succeed.
 */
int setRotation(unsigned int size);

/**
 * Rotate log file if it exceeds maximum size
 * @param filename Path to log file
 * @return 1 if rotated, 0 if not needed, error code on failure
 */
int rotate(const char * filename);

/**
 * Set the global default log file. Function @logger
 * and @logdual will use this file as the output log file
 * @param filename Default target log file path
 */
int setDefaultLogFile(const char * filename);

/**
 * Set the global default xfd and color. Function @logxfd
 * and @logdual will use this xfd and color as the output xfd and color
 * @param xfd Default target xfd
 * @param color Default option for output in color
 */
int setDefaultXfdAndColor(const xfd_t xfd, bool color);

/**
 * Write formatted message to *default* log file with a buffer
 * @param level Log level severity
 * @param format printf-style format string
 * @param ... Variable arguments for format string
 * @return 0 on success, error code on failure
 */
int logger(LogLevel level, const char * format, ...);

/**
 * Write formatted message to *specified* log file with a buffer
 * @param level Log level severity
 * @param filename Target log file path
 * @param format printf-style format string
 * @param ... Variable arguments for format string
 * @return 0 on success, error code on failure
 */
int log2file(LogLevel level,
             const char * filename,
             const char * format, ...);

/**
 * Write formatted message to *default* xfd right now
 * @param level Log level severity
 * @param format printf-style format string
 * @param ... Variable arguments for format string
 * @return 0 on success, error code on failure
 */
int logxfd(LogLevel level, const char * format, ...);

/**
 * Write formatted message to *specified* xfd right now
 * @param level Log level severity
 * @param xfd Target xfd
 * @param color true for output in color
 * @param format printf-style format string
 * @param ... Variable arguments for format string
 * @return 0 on success, error code on failure
 */
int log2xfd(LogLevel level,
            xfd_t xfd,
            bool color,
            const char * format, ...);

/**
 * Write formatted message to *default* dual routes
 * @param level Log level severity
 * @param format printf-style format string
 * @param ... Variable arguments for format string
 * @return Each 0 on success, error code on failure
 */
DualReturn logdual(LogLevel level, const char * format, ...);

/**
 * Write formatted message to *specified* dual routes
 * @param level Log level severity
 * @param dual Target dual routes
 * @param format printf-style format string
 * @param ... Variable arguments for format string
 * @return Each 0 on success, error code on failure
 */
DualReturn log2dual(LogLevel level,
             const LogDual * dual,
             const char * format, ...);


/** Some quick-to-use macros */
/** Output logs to standard output */
#define log2out(level, format, ...) \
    log2xfd((level), XFD_STDOUT, true, (format), ##__VA_ARGS__)

/** Output logs to standard error */
#define log2err(level, format, ...) \
    log2xfd((level), XFD_STDERR, true, (format), ##__VA_ARGS__)

/** Automatically convert file name + line number to string */
#define HERE " at " __FILE__ ":" toString(__LINE__)


/**
 * Force flush all buffered logs to disk
 * @return 0 on success, error code on failure
 */
int flush(void);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // LOG_FILE_WRITE
