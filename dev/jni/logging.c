#include "logging.h"

void log_message(LogLevel level, const char *format, ...) {

    if (level < MIN_LOG_LEVEL) return;

    time_t raw_time;
    struct tm *time_info;
    char time_buffer[20];

    time(&raw_time);


    time_info = localtime(&raw_time);
    if (!time_info) {
        return;
    }
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

    const char *level_str;
    switch (level) {
    case LOG_DEBUG:
        level_str = "DEBUG";
        break;
    case LOG_INFO:
        level_str = "INFO ";
        break;
    case LOG_WARNING:
        level_str = "WARN ";
        break;
    case LOG_ERROR:
        level_str = "ERRO ";
        break;
    case LOG_NO:
        return;
    default:
        level_str = "UNKNOWN";
    }

    printf("[%s] [%s] ", time_buffer, level_str);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");
}