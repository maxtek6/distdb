#ifndef _WIN32

#include <fmap.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct fmap 
{
    void *data;
    size_t size;
};

struct fmap* fmap_open(const char *path)
{
    struct fmap *map = malloc(sizeof(struct fmap));
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        goto error;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        goto error;
    }
    map->size = st.st_size;
    map->data = mmap(NULL, map->size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map->data == MAP_FAILED)
    {
        goto error;
    }

    goto done;
error:
    free(map);
    map = NULL;
done:
    if (fd >= 0)
        close(fd);
    return map;
}

void fmap_close(struct fmap *map)
{
    if (map)
    {
        if (map->data && map->size)
            munmap(map->data, map->size);
        free(map);
    }
}

const void *fmap_data(const struct fmap *map)
{
    return map ? map->data : NULL;
}

size_t fmap_size(const struct fmap *map)
{
    return map ? map->size : 0;
}

#endif