#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_NO
} LogLevel;

#ifndef MIN_LOG_LEVEL
#define MIN_LOG_LEVEL LOG_INFO
#endif

void log_message(LogLevel level, const char *format, ...);

#endif