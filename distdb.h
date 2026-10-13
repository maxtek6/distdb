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

/**
 * @brief Opens a distdb archive from the specified path.
 * 
 * @param path The file path to the distdb archive.
 * @param db Pointer to a pointer that will receive the opened archive.
 * @return distdb_status DISTDB_SUCCESS on success, DISTDB_ERROR_INVALID_ARGUMENT if one or more of the 
 * arguments are invalid, or DISTDB_ERROR_ARCHIVE_UNREADABLE if the archive could not be read.
 */
distdb_status distdb_open(const char *path, struct distdb_archive **db);

/**
 * @brief Closes a previously opened distdb archive.
 * 
 * @param db Pointer to the archive. If the archive is NULL, the function 
 * does nothing.
 */
void distdb_close(struct distdb_archive *db);

/**
 * @brief Finds a resource in the archive by path.
 * 
 * @param db Pointer to the archive.
 * @param path The path of the resource to find.
 * @param resource Pointer to a structure that will receive the resource data.
 * @return distdb_status DISTDB_SUCCESS on success, DISTDB_ERROR_RESOURCE_NOT_FOUND if 
 * the resource is not found, or DISTDB_ERROR_INVALID_ARGUMENT if one or more of the 
 * arguments are invalid.
 */
distdb_status distdb_find(const struct distdb_archive *db, const char *path, struct distdb_resource *resource);

/**
 * @brief Lists all resource paths in the archive.
 * 
 * @param db Pointer to the archive.
 * @param paths Array that will receive the resource paths. If NULL, the function will only set count.
 * @param count If paths is NULL, this will receive the number of resources. If paths is not NULL, it 
 * will be used as the max number of paths to write to the array, and will be updated with the actual 
 * number of paths written to the array.
 * @return distdb_status DISTDB_SUCCESS on success, DISTDB_ERROR_INVALID_ARGUMENT if one or more of the 
 * arguments are invalid.
 */
distdb_status distdb_list(const struct distdb_archive *db, const char **paths, size_t *count);

#ifdef __cplusplus
}
#endif

#endif // DISTDB_H