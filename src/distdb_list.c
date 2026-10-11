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
    if (distdb_check_args(2, archive, size))
    {
        if (paths)
            *size = distdb_list_fill(archive, paths, *size);
        else
            distdb_list_count(archive, size);
        result = DISTDB_SUCCESS;
    }
    else
    {
        result = DISTDB_ERROR_INVALID_ARGUMENT;
    }
    return result;
}