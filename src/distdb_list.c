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

#include <distdb_common.h>

static void distdb_list_count(const struct distdb_archive *archive, size_t *size)
{
    *size = distdb_Resource_vec_len(archive->resources);
}

static size_t distdb_list_fill(const struct distdb_archive *archive, const char ** paths, size_t max_size)
{
    size_t result = distdb_Resource_vec_len(archive->resources);
    
    if (result > max_size)
    {
        result = max_size;
    }

    for (size_t index = 0; index < result; index++)
    {
        *(paths + index) = distdb_Resource_path_get(distdb_Resource_vec_at(archive->resources, index));
    }

    return result;
}

distdb_status distdb_list(const struct distdb_archive *archive, const char ** paths, size_t *size)
{
    distdb_status result;
    if (distdb_check_args(&result, 2, archive, size))
    {
        if (paths)
            *size = distdb_list_fill(archive, paths, *size);
        else
            distdb_list_count(archive, size);
    }
    return result;
}