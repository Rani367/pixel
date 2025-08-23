#ifndef PH_LOG_H
#define PH_LOG_H

#include "common.h"

typedef enum {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
} LogLevel;

// Set the minimum log level (messages below this level are ignored)
void log_set_level(LogLevel level);

// Set the output file for log messages (default: stderr)
void log_set_output(FILE* out);

// Internal logging function (use the macros instead)
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 4, 5)))
#endif
void log_write(LogLevel level, const char* file, int line, const char* fmt, ...);

// Logging macros
#ifdef NDEBUG
    // In release builds, disable trace and debug logging
    #define LOG_TRACE(...) ((void)0)
    #define LOG_DEBUG(...) ((void)0)
#else
    #define LOG_TRACE(...) log_write(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
    #define LOG_DEBUG(...) log_write(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#endif

#define LOG_INFO(...)  log_write(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  log_write(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log_write(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS_