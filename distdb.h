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

#ifndef DISTDB_H
#define DISTDB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

typedef enum distdb_status
{
    DISTDB_SUCCESS = 0,
    DISTDB_ERROR_INVALID_ARGUMENT,
    DISTDB_ERROR_MMAP_FAILED,
    DISTDB_ERROR_ARCHIVE_UNREADABLE,
    DISTDB_ERROR_RESOURCE_NOT_FOUND,
} distdb_status;

struct distdb_resource
{
    const char *mime_type;
    const void *content;
    size_t length;
};
struct distdb_archive;

distdb_status distdb_open(const char *path, struct distdb_archive **db);

void distdb_close(struct distdb_archive *db);

distdb_status distdb_find(const struct distdb_archive *db, const char *path, struct distdb_resource *resource);

distdb_status distdb_list(const struct distdb_archive *db, const char **paths, size_t *count);

#ifdef __cplusplus
}
#endif

#endif // DISTDB_H