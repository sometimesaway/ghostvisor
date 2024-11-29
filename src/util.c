#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "util.h"

static const char *log_level_to_string(log_level_t level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARN: return "WARN";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void log_message(log_level_t level, const char *format, ...) {
    va_list args;
    char buffer[256];
    time_t now;
    struct tm *local_time;

    time(&now);
    local_time = localtime(&now);

    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d [%s] ",
             local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
             log_level_to_string(level));

    fputs(buffer, stderr);

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fputc('\n', stderr);
}
