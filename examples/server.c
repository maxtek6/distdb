#include <distdb.h>

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>

#include <signal.h>
#include <stdio.h>
#include <string.h>

static struct event_base *base;

static int serve_content(const char *archivePath);

static const char *resolve_path(struct evhttp_request *req);

static void evhttp_callback(struct evhttp_request *req, void *arg);

static void handle_signal(int signum);

int main(int argc, const char **argv)
{
    int rc;
    if (argc > 1)
    {
        base = event_base_new();
        rc = serve_content(argv[1]);
        event_base_free(base);
    }
    else
    {
        fputs("usage: server <archive path>\n", stderr);
        rc = 1;
    }
    return rc;
}

int serve_content(const char *archivePath)
{
    struct evhttp *http;
    struct distdb_archive *archive;
    int rc;

    rc = 1; // Default to failure
    if (distdb_open(archivePath, &archive) == 0)
    {
        http = evhttp_new(base);
        if (http != NULL)
        {
            if (evhttp_bind_socket(http, "0.0.0.0", 12345) == 0)
            {
                evhttp_set_gencb(http, evhttp_callback, archive);
                signal(SIGINT, handle_signal);
                signal(SIGTERM, handle_signal);
                rc = event_base_dispatch(base);
            }
            evhttp_free(http);
        }
        distdb_close(archive);
    }
    return rc;
}

const char *resolve_path(struct evhttp_request *req)
{
    const struct evhttp_uri *uri = evhttp_request_get_evhttp_uri(req);
    const char *result = evhttp_uri_get_path(uri);
    if (strlen(result) == 0 || (strlen(result) == 1 && result[0] == '/'))
    {
        result = "/index.html";
    }
    return result;
}

void evhttp_callback(struct evhttp_request *req, void *arg)
{
    struct distdb_archive *archive = (struct distdb_archive *)arg;
    enum evhttp_cmd_type cmd = evhttp_request_get_command(req);
    if (cmd == EVHTTP_REQ_GET)
    {
        char length_string[32];
        struct distdb_resource resource;
        const int rc = distdb_find(archive, resolve_path(req), &resource);
        if (rc == DISTDB_SUCCESS)
        {
            struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
            struct evbuffer *buf = evhttp_request_get_output_buffer(req);
            evhttp_add_header(headers, "Content-Type", resource.mime_type);
            snprintf(length_string, sizeof(length_string), "%zu", resource.length);
            evhttp_add_header(headers, "Content-Length", length_string);
            evbuffer_add_reference(buf, resource.content, resource.length, NULL, NULL);
            evhttp_send_reply(req, HTTP_OK, "OK", buf);
        }
        else if (rc == DISTDB_ERROR_RESOURCE_NOT_FOUND)
        {
            evhttp_send_error(req, HTTP_NOTFOUND, "Resource not found");
        }
        else
        {
            evhttp_send_error(req, HTTP_INTERNAL, "Internal server error");
        }
    }
    else
    {
        evhttp_send_error(req, HTTP_BADMETHOD, "Unsupported HTTP method");
    }
}

void handle_signal(int sig)
{
    event_base_loopbreak(base);
}