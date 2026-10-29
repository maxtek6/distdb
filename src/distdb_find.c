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

distdb_status distdb_find(const struct distdb_archive *db, const char *path, struct distdb_resource *resource)
{
    distdb_status result;
    if (distdb_check_args(&result, 3, db, path, resource))
    {
        const size_t index = distdb_Resource_vec_find_by_path(db->resources, path);
        if (index != (size_t)-1)
        {
            const distdb_Resource_table_t item = distdb_Resource_vec_at(db->resources, index);
            resource->mime_type = distdb_Resource_type_get(item);
            resource->content = db->data + distdb_Resource_offset_get(item);
            resource->length = distdb_Resource_length_get(item);
        }
        else
        {
            result = DISTDB_ERROR_RESOURCE_NOT_FOUND;
        }
    }
    return result;
}