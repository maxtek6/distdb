#include <eventx.h>

#include <event2/thread.h>

#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

void event_platform_init()
{
    evthread_use_pthreads();
}

void event_platform_shutdown()
{
    libevent_global_shutdown();
}

static size_t hardware_concurrency()
{
    return (size_t)sysconf(_SC_NPROCESSORS_ONLN);
}

static void *thread_entry(void *arg)
{
    struct event_base *base = (struct event_base *)arg;
    (void)event_base_dispatch(base);
    return NULL;
}

struct event_group_entry
{
    struct event_base *base;
    pthread_t thread;
};

struct event_group
{
    struct event_group_entry *entries;
    size_t count;
};

struct event_group *event_group_create(size_t count)
{
    struct event_group *group = malloc(sizeof(struct event_group));
    if (count > 0)
    {
        group->count = count;
    }
    else
    {
        group->count = hardware_concurrency();
    }
    group->entries = calloc(group->count, sizeof(struct event_group_entry));
    for(size_t index = 0; index < group->count; index++)
    {
        (group->entries + index)->base = event_base_new();
    }
    return group;
}

void event_group_free(struct event_group *group)
{
    for (size_t index = 0; index < group->count; index++)
    {
        event_base_free((group->entries + index)->base);
    }
    free(group->entries);
    free(group);
}


struct event_base *event_group_base(struct event_group *group, size_t index)
{
    struct event_base *base;
    if (index < group->count)
    {
        base = (group->entries + index)->base;
    }
    else
    {
        base = NULL;
    }
    return base;
}

size_t event_group_count(struct event_group *group)
{
    return group->count;
}

void event_group_start(struct event_group *group)
{
    for (size_t index = 0; index < group->count; index++)
    {
        pthread_create(&(group->entries + index)->thread, NULL, thread_entry, (group->entries + index)->base);
    }
}

void event_group_stop(struct event_group *group)
{
    for (size_t index = 0; index < group->count; index++)
    {
        fputs("stopping event loop\n", stderr);
        event_base_loopbreak((group->entries + index)->base);
    }
}

void event_group_wait(struct event_group *group)
{
    for (size_t index = 0; index < group->count; index++)
    {
        pthread_join((group->entries + index)->thread, NULL);
    }
}