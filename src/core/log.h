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
void log_set_level(LogLevel lev