#include <distdb_common.h>

distdb_status distdb_find(const struct distdb_archive *db, const char *path, struct distdb_resource *resource)
{
    distdb_status result;
    if (distdb_check_args(3, db, path, resource))
    {
        const size_t index = distdb_Resource_vec_find_by_path(db->resources, path);
        if (index != (size_t)-1)
        {
            const distdb_Resource_table_t item = distdb_Resource_vec_at(db->resources, index);
            resource->mime_type = distdb_Resource_type_get(item);
            resource->content = db->data + distdb_Resource_offset_get(item);
            resource->length = distdb_Resource_length_get(item);
            result = DISTDB_SUCCESS;
        }
        else
        {
            result = DISTDB_ERROR_RESOURCE_NOT_FOUND;
        }
    }
    else
    {
        result = DISTDB_ERROR_INVALID_ARGUMENT;
    }
    return result;
}