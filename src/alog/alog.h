#ifndef ALOG_H
#define ALOG_H

enum alog_level
{
    ALOG_LEVEL_ERROR,
    ALOG_LEVEL_WARN,
    ALOG_LEVEL_INFO,
    ALOG_LEVEL_DEBUG
};

struct alog_mutex;

struct alog_mutex *alog_mutex_new();
void alog_mutex_free(struct alog_mutex *mutex);
void alog_mutex_lock(struct alog_mutex *mutex);
void alog_mutex_unlock(struct alog_mutex *mutex);

struct alog_logger;

struct alog_logger *alog_open(const char *name, enum alog_level level, const char *path);

void alog_close(struct alog_logger *logger);

void alog_logf(struct alog_logger *logger, enum alog_level level, const char *fmt, ...);

#define ALOG_ERROR(logger, fmt, ...) alog_logf(logger, ALOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define ALOG_WARN(logger, fmt, ...) alog_logf(logger, ALOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define ALOG_INFO(logger, fmt, ...) alog_logf(logger, ALOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define ALOG_DEBUG(logger, fmt, ...) alog_logf(logger, ALOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)

#endif // ALOG_H