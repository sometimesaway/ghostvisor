#ifndef UTIL_H
#define UTIL_H

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level_t;

void log_message(log_level_t level, const char *format, ...);

#define log_debug(fmt, ...) log_message(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  log_message(LOG_INFO, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)  log_message(LOG_WARN, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log_message(LOG_ERROR, fmt, ##__VA_ARGS__)

#endif // UTIL_H
