#ifndef LOG_FILE_WRITE
#define LOG_FILE_WRITE

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
    LOG_NOTIP     ///< Special level - no prefix/suffix formatting
} LogLevel;

/**
 * Suggested log rotation sizes
 * Provides predefined size options for log rotation
 */
typedef enum {
    ROT_DEFAULT  = 64,      ///< Default size (64MB)
    ROT_HIGHFRE  = 32,      ///< For high frequency logs (32MB)
    ROT_ROUTINE  = 256,     ///< For routine application logs (256MB)
    ROT_MEDIUML  = 1024,    ///< For medium/large services (1GB)
    ROT_BIGDATA  = 64*1024, ///< For big data applications (64GB)
    ROT_MAGNANI  = 512*1024 ///< For very large systems (512GB)
} RotationRange;

/**
 * Set maximum log file size before rotation
 * @param size Size in megabytes (1MB to 1PB)
 * @return 0 on success, error code on failure
 */
int setRotation(unsigned int size);

/**
 * Rotate log file if it exceeds maximum size
 * @param filename Path to log file
 * @return 1 if rotated, 0 if not needed, error code on failure
 */
int rotateLog(const char * filename);

/**
 * Write formatted message to log file
 * @param level Log level severity
 * @param filename Target log file path
 * @param format printf-style format string
 * @param ... Variable arguments for format string
 * @return 0 on success, error code on failure
 */
int logMessage(LogLevel level, const char * filename, const char * format, ...);

/**
 * Force flush all buffered logs to disk
 * @return 0 on success, error code on failure
 */
int flushBuffer();

#endif // LOG_FILE_WRITE
