#include "log.h"
#include <stdarg.h>
#include <time.h>

static LogLevel current_level = LOG_INFO;
static FILE* log_output = NULL;

static const char* level_names[] = {
    [LOG_TRACE] = "TRACE",
    [LOG_DEBUG] = "DEBUG",
    [LOG_INFO]  = "INFO",
    [LOG_WARN]  = "WARN",
    [LOG_ERROR] = "ERROR",
};

void log_set_level(LogLevel level) {
    current_level = level;
}

void log_set_output(FILE* out) {
    log_output = out;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
void log_write(LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (level < current_level) {
        return;
    }

    FILE* out = log_output ? log_output : stderr;

    // Get current time
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "s, fmt);
    vfprintf(out, fmt, args);
    va_end(args);

    fprintf(out, "\n");
    fflush(out);
}
#pragma GCC diagnostic pop
%Y-%m-%d %H:%M:%S", tm_info);

    // Extract just the filename from the path
    const char* filename = file;
    for (const char* p = file; *p; p++) {
        if (*p == '/' || *p == '\\') {
            filename = p + 1;
        }
    }

    // Print log header
    fprintf(out, "%s [%s] %s:%d: ", time_buf, level_names[level], filename, line);

    // Print the message
    va_list args;
    va_start(arg