#include <distdb_common.h>

static bool distdb_open_map(struct distdb_archive *archive, const char *path)
{
    archive->map = fmap_open(path);
    return (archive->map != NULL);
}

static bool distdb_open_parse(struct distdb_archive *archive)
{
    const void *map_data = fmap_data(archive->map);
    bool result = false;
    const size_t archive_size = fmap_size(archive->map);
    int rc = distdb_Archive_verify_as_root_with_identifier_and_size(
        map_data, archive_size, distdb_Archive_identifier);
    if (rc == flatcc_verify_ok)
    {
        size_t header_size = 0;
        void *header = flatbuffers_read_size_prefix((void*)map_data, &header_size);
        const distdb_Archive_table_t root = distdb_Archive_as_root(header);
        archive->data = (const uint8_t*)map_data + header_size + sizeof(flatbuffers_uoffset_t);
        archive->resources = distdb_Archive_resources_get(root);
        result = true;
    }
    else
    {
        printf("parse error %d: %s\n", rc, flatcc_verify_error_string(rc));
    }
    return result;
}

distdb_status distdb_open(const char *path, struct distdb_archive **archive)
{
    distdb_status result;
    if (!distdb_check_args(2, path, archive))
    {
        result = DISTDB_ERROR_INVALID_ARGUMENT;
        goto done;
    }

    *archive = malloc(sizeof(struct distdb_archive));
    if (!distdb_open_map(*archive, path))
    {
        result = DISTDB_ERROR_MMAP_FAILED;
        goto error;
    }

    if (!distdb_open_parse(*archive))
    {
        result = DISTDB_ERROR_ARCHIVE_UNREADABLE;
        goto error;
    }

    result = DISTDB_SUCCESS;
    goto done;
error:
    distdb_close(*archive);
    *archive = NULL;
done:
    return result;
}

void distdb_close(struct distdb_archive *archive)
{
    if (archive)
    {
        if (archive->map)
            fmap_close(archive->map);
        free(archive);
    }
}