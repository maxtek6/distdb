#ifndef FMAP_H
#define FMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

struct fmap;

struct fmap* fmap_open(const char *path);

void fmap_close(struct fmap *map);

const void *fmap_data(const struct fmap *map);

size_t fmap_size(const struct fmap *map);

#ifdef __cplusplus
}
#endif

#endif // FMAP_H