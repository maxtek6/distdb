#include <alog.h>

#include <stdlib.h>
#include <pthread.h>

struct alog_mutex
{
    pthread_mutex_t mutex;
};

struct alog_mutex *alog_mutex_new()
{
    struct alog_mutex *m = malloc(sizeof(struct alog_mutex));
    pthread_mutex_init(&m->mutex, NULL);
    return m;
}

void alog_mutex_free(struct alog_mutex *m)
{
    pthread_mutex_destroy(&m->mutex);
    free(m);
}

void alog_mutex_lock(struct alog_mutex *m)
{
    pthread_mutex_lock(&m->mutex);
}

void alog_mutex_unlock(struct alog_mutex *m)
{
    pthread_mutex_unlock(&m->mutex);
}
