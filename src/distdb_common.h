/*
 * Copyright (c) 2026 Maxtek Consulting
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DISTDB_COMMON_H
#define DISTDB_COMMON_H

#include <distdb.h>
#include <schema_verifier.h>

#include <fmap.h>

#include <stdarg.h>
#include <stdbool.h>

struct distdb_archive
{
    struct fmap *map;
    distdb_Resource_vec_t resources;
    const uint8_t *data;
};

static inline bool distdb_check_args(distdb_status *status, size_t count, ...)
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
    *status = result ? DISTDB_SUCCESS : DISTDB_ERROR_INVALID_ARGUMENT;
    return result;
}

#endif // DISTDB_COMMON_H