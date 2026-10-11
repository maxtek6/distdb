#ifndef DISTDB_COMMON_H
#define DISTDB_COMMON_H

#include <distdb.h>
#include <schema_verifier.h>

#include <fmap.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

struct distdb_archive
{
    struct fmap *map;
    distdb_Resource_vec_t resources;
    const uint8_t *data;
};

static inline bool distdb_check_args(size_t count, ...)
{
    va_list args;
    va_start(args, count);
    const void *item;

    bool result = true;
    size_t index = 0;
    while (result && index < count)
    {
        item = va_arg(args, const void*);
        result = (item != NULL);
        index++;
    }
    va_end(args);
    return result;
}

#endif // DISTDB_COMMON_H