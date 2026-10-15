#include <eventx.h>

#include <sys/socket.h>

#include <event2/listener.h>

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

struct event_http_entry
{
    struct evhttp *http;
    int reserved_for_tls;
};

static int evhttp_entry_create(struct event_http_entry *entry, struct event_base *base, const struct sockaddr *addr, int socklen)
{
    const int optval = 1;
    struct evconnlistener *listener;
    struct evhttp_bound_socket *bound_socket;
    int result;

    evutil_socket_t fd = socket(addr->sa_family, SOCK_STREAM, 0);

    if (fd < 0)
    {
        result = -1;
        goto error;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 || 
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0)
    {
        result = -2;
        goto error;
    }

    if (bind(fd, addr, (socklen_t)socklen) < 0)
    {
        fprintf(stderr, "Failed to bind socket(%d)\n", errno);
        result = -3;
        goto error;
    }

    evutil_make_socket_nonblocking(fd);

    listener = evconnlistener_new(base, NULL, NULL, LEV_OPT_CLOSE_ON_FREE, -1, fd);
    if (listener == NULL)
    {
        result = -4;
        goto error;
    }

    entry->http = evhttp_new(base);
    bound_socket = evhttp_bind_listener(entry->http, listener);
    if (bound_socket == NULL)
    {
        result = -5;
        goto error;
    }

    result = 0;
    goto done;
error:
    if (listener)
        evconnlistener_free(listener);
    else if (fd >= 0)
        evutil_closesocket(fd);
done:
    return result;
}

void evhttp_entry_destroy(struct event_http_entry *entry)
{
    if (entry->http)
        evhttp_free(entry->http);
}

struct event_http
{
    struct sockaddr_storage addr;
    int socklen;
    struct event_http_entry *entries;
    size_t count;
};

struct event_http *event_http_new(
    const char *address, 
    const char *tls_cert, 
    const char *tls_key)
{
    struct event_http *server = malloc(sizeof(struct event_http));
    server->socklen = sizeof(struct sockaddr_storage);
    server->entries = NULL;
    server->count = 0;
    if (evutil_parse_sockaddr_port(address, (struct sockaddr *)&server->addr, &server->socklen) < 0)
    {
        free(server);
        server = NULL;        
    }
    return server;
}

void event_http_free(struct event_http *server)
{
    if (server)
    {
        if (server->count > 0)
        {
            for (size_t i = 0; i < server->count; i++)
                evhttp_entry_destroy(&server->entries[i]);
            free(server->entries);
        }
        free(server);
    }
}

int event_http_listen(struct event_http *server, struct event_base *base, void (*callback)(struct evhttp_request *req, void *arg), void *arg)
{
    int result;
    size_t index = server->count;
    server->entries = realloc(server->entries, sizeof(struct event_http_entry) * (index + 1));
    result = evhttp_entry_create(&server->entries[index], base, (struct sockaddr *)&server->addr, server->socklen);
    if (result == 0)
    {
        evhttp_set_gencb(server->entries[index].http, callback, arg);
    }
    server->count++;
    return result;
}
