#include <fmap.h>

#include <mio/mmap.hpp>

#include <memory>

struct fmap : public mio::basic_mmap_source<uint8_t>
{

};

struct fmap* fmap_open(const char *path)
{
    std::error_code ec;
    fmap *result(nullptr);
    std::unique_ptr<fmap> map(new fmap());
    map->map(path, ec);
    if (map->is_mapped())
        result = map.release();
    return result;
}

void fmap_close(struct fmap *map)
{
    delete map;
}

const void *fmap_data(const struct fmap *map)
{
    return map->data();
}

size_t fmap_size(const struct fmap *map)
{
    return map->size();
}