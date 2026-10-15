#include <alog.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct alog_logger
{
    struct alog_mutex *mutex;
    const char *name;
    enum alog_level level;
    FILE *file;
    int (*close_func)(FILE *file);
};

struct alog_logger *alog_open(const char *name, enum alog_level level, const char *path)
{
    struct alog_logger *logger = malloc(sizeof(struct alog_logger));
    logger->mutex = alog_mutex_new();
    logger->name = name;
    logger->level = level;
    if (strcmp(path, "stderr") == 0)
    {
        logger->file = stderr;
        logger->close_func = NULL;
    }
    else if (strcmp(path, "stdout") == 0)
    {
        logger->file = stdout;
        logger->close_func = NULL;
    }
    else
    {
        logger->file = fopen(path, "wa");
        if (logger->file)
        {
            logger->close_func = fclose;
        }
        else
        {
            alog_close(logger);
            logger = NULL;
        }
    }
    return logger;
}

void alog_close(struct alog_logger *logger)
{
    if (logger)
    {
        if (logger->close_func)
        {
            logger->close_func(logger->file);
        }
        alog_mutex_free(logger->mutex);
        free(logger);
    }
}

void alog_logf(struct alog_logger *logger, enum alog_level level, const char *fmt, ...)
{
    if (level <= logger->level)
    {
        alog_mutex_lock(logger->mutex);
        fprintf(logger->file, "[%s] ", logger->name);
        va_list args;
        va_start(args, fmt);
        vfprintf(logger->file, fmt, args);
        va_end(args);
        fprintf(logger->file, "\n");
        alog_mutex_unlock(logger->mutex);
    }
}