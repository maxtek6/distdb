#include <distdb.h>

#include <alog.h>

#include <eventx.h>
#include <event2/buffer.h>

#include <argtable3.h>

#include <signal.h>


struct distdbd_server
{
    struct alog_logger *logger;
    struct distdb_archive *archive;
    struct event_group *event_group;
    struct event_http *event_http;
};

static struct distdbd_server g_server;

static int distdbd_init(int argc, char **argv);
static int distdbd_init_logger(const char *logger_path, int debug);
static int distdbd_init_server(const char *archive_path, const char *address, size_t threads);
static void distdbd_http_callback(struct evhttp_request *req, void *arg);

static void distdbd_run();
static void distdbd_signal(int signum);

static void distdbd_cleanup();

int main(int argc, char **argv)
{
    event_platform_init();
    int result = distdbd_init(argc, argv);
    if (result == 0)
    {
        distdbd_run();
    }
    distdbd_cleanup();
    event_platform_shutdown();
    return result;
}

int distdbd_init(int argc, char **argv)
{
    int result = 0;
    struct arg_lit *help = arg_litn("h", "help", 0, 1,"Display this help and exit");
    struct arg_file *archive_path = arg_filen(NULL, NULL, "<file>", 1, 1, "Path to the archive file");
    struct arg_str *address = arg_strn("a", "address", "<string>", 1, 1, "Address to bind the server to");
    struct arg_file *logger_path = arg_filen("l", "log", "<file>", 0, 1, "Path to the logger file");
    struct arg_lit *debug = arg_lit0("d", "debug", "Enable debug mode");
    struct arg_int *threads = arg_intn("t", "threads", "<n>", 0, 1,"Number of threads to use");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, archive_path, address, logger_path, debug, threads, end};

    int nerrors = arg_parse(argc, argv, argtable);
    if (help->count > 0)
    {
        fprintf(stderr, "usage: %s", argv[0]);
        arg_print_syntax(stderr, argtable, "\n");
        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
    }
    else if (nerrors > 0)
    {
        fprintf(stderr, "error: Invalid arguments\n");
        arg_print_errors(stderr, end, argv[0]);
        result = 1;
    }
    else
    {
        result = distdbd_init_logger(logger_path->count > 0 ? logger_path->filename[0] : "stderr", debug->count);
        if (result == 0)
        {
            result = distdbd_init_server(archive_path->filename[0], address->sval[0], threads->count > 0 ? threads->ival[0] : 0);
        }
    }
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return result;

}

void distdbd_run()
{
    if (g_server.event_group)
    {
        ALOG_INFO(g_server.logger, "starting event group");
        event_group_start(g_server.event_group);
        signal(SIGINT, distdbd_signal);
        signal(SIGTERM, distdbd_signal);
        ALOG_INFO(g_server.logger, "waiting for event group to complete");
        event_group_wait(g_server.event_group);
    }
}

void distdbd_signal(int signum)
{
    ALOG_INFO(g_server.logger, "received signal %d, stopping event group", signum);
    event_group_stop(g_server.event_group);
}

void distdbd_cleanup()
{
    if (g_server.event_http)
    {
        event_http_free(g_server.event_http);
    }
    if (g_server.event_group)
    {
        event_group_free(g_server.event_group);
    }
    if (g_server.archive)
    {
        distdb_close(g_server.archive);
    }
    if (g_server.logger)
    {
        alog_close(g_server.logger);
    }
}

int distdbd_init_logger(const char *logger_path, int debug)
{
    enum alog_level level = (debug > 0) ? ALOG_LEVEL_DEBUG : ALOG_LEVEL_INFO;
    g_server.logger = alog_open("distdbd", level, logger_path);
    return g_server.logger ? 0 : 1;
}

int distdbd_init_server(const char *archive_path, const char *address, size_t threads)
{
    int result;
    ALOG_INFO(g_server.logger, "opening archive %s", archive_path);
    result = distdb_open(archive_path, &g_server.archive);
    if (result == DISTDB_SUCCESS)
    {
        ALOG_INFO(g_server.logger, "starting HTTP server on address %s", address);
        g_server.event_http = event_http_new(address, NULL, NULL);
        if (g_server.event_http != NULL)
        {
            ALOG_INFO(g_server.logger, "HTTP server created successfully");
            g_server.event_group = event_group_create(threads);
            for (size_t index = 0; index < event_group_count(g_server.event_group); index++)
            {
                if (result == 0)
                    result = event_http_listen(g_server.event_http, event_group_base(g_server.event_group, index), distdbd_http_callback, NULL);
            }
        }
    }
    return result;
}

void distdbd_http_callback(struct evhttp_request *req, void *arg)
{
    if (evhttp_request_get_command(req) == EVHTTP_REQ_GET)
    {
        const char *path = evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req));
        struct distdb_resource resource;
        ALOG_DEBUG(g_server.logger, "received GET request for path: %s", path);
        const int result = distdb_find(g_server.archive, path, &resource);
        if (result == DISTDB_SUCCESS)
        {
            char length_string[10];
            struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
            struct evbuffer *output = evhttp_request_get_output_buffer(req);
            snprintf(length_string, sizeof(length_string), "%zu", resource.length);
            evhttp_add_header(headers, "Content-Type", resource.mime_type);
            evhttp_add_header(headers, "Content-Length", length_string);
            evbuffer_add_reference(output, resource.content, resource.length, NULL, NULL);
            evhttp_send_reply(req, HTTP_OK, "OK", output);
        } 
        else if (result == DISTDB_ERROR_RESOURCE_NOT_FOUND)
        {
            ALOG_DEBUG(g_server.logger, "resource not found: %s", path);
            evhttp_send_error(req, HTTP_NOTFOUND, "Resource not found");
        }
        else
        {
            ALOG_ERROR(g_server.logger, "error retrieving resource: %s", path);
            evhttp_send_error(req, HTTP_INTERNAL, "Internal server error");
        }
    }
    else
    {
        evhttp_send_error(req, HTTP_BADMETHOD, "Method not allowed");
    }
}