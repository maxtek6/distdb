#ifndef EVENTX_H
#define EVENTX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <event2/event.h>
#include <event2/http.h>

void event_platform_init();

void event_platform_shutdown();

struct event_http;

struct event_http *event_http_new(
    const char *address, 
    const char *tls_cert, 
    const char *tls_key);

void event_http_free(struct event_http *server);

int event_http_listen(struct event_http *server, struct event_base *base, void (*callback)(struct evhttp_request *req, void *arg), void *arg);


struct event_group;

struct event_group *event_group_create(size_t count);

struct event_base *event_group_base(struct event_group *group, size_t index);

size_t event_group_count(struct event_group *group);

void event_group_free(struct event_group *group);

void event_group_start(struct event_group *group);

void event_group_stop(struct event_group *group);

void event_group_wait(struct event_group *group);

#ifdef __cplusplus
}
#endif

#endif // EVENTX_H