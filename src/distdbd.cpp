#include <condition_variable>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <variant>
#include <vector>

#include <csignal>

#include <distdb.h>

#include <argparse/argparse.hpp>

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/thread/latch.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>

static std::ofstream g_sink_file;
static std::shared_ptr<spdlog::logger> g_logger;

static bool create_logger(const std::string &path, bool debug)
{
    spdlog::level::level_enum level = debug ? spdlog::level::debug : spdlog::level::info;
    bool result(false);
    std::shared_ptr<spdlog::sinks::ostream_sink_mt> sink;

    if (path == "stderr")
    {
        sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cerr);
    }
    else if (path == "stdout")
    {
        sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout);
    }
    else
    {
        g_sink_file.open(path, std::ios::out | std::ios::app);
        if (g_sink_file.is_open())
            sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(g_sink_file);
    }

    if (sink)
    {
        g_logger = std::make_shared<spdlog::logger>("logger", sink);
        g_logger->set_level(level);
    }

    return sink != nullptr;
}

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = boost::beast::http;
using net = boost::asio::ip::tcp;
using latch_type = boost::latch;

using http_result = std::variant<std::monostate, distdb_resource, http::status>;

class http_session : std::enable_shared_from_this<http_session>
{
private:
    static std::unique_ptr<distdb_archive, decltype(&distdb_close)> _archive;
    beast::tcp_stream _stream;
    beast::flat_buffer _buffer;
    http::request<http::empty_body> _request;

    void start_read()
    {
        auto self = shared_from_this();
        g_logger->debug("http_session::start_read(): starting async read");
        _stream.expires_after(std::chrono::seconds(30));
        http::async_read(_stream, _buffer, _request, [self](boost::system::error_code ec, std::size_t bytes_transferred)
                         {
            if (ec)
                g_logger->error("http_session::start_read(): async read failed: {}", ec.message());
            else
            {
                g_logger->debug("http_session::start_read(): transferred {} bytes, processing request", bytes_transferred);
                self->handle_request();
            } });
    }

    template <typename BodyType>
    void start_write(const std::shared_ptr<http::response<BodyType>> &response)
    {
        auto self = shared_from_this();
        g_logger->debug("http_session::start_write(): starting async write");
        _stream.expires_after(std::chrono::seconds(30));
        http::async_write(_stream, *response, [self](boost::system::error_code ec, std::size_t bytes_transferred)
                          {
            if (ec)
                g_logger->error("http_session::start_write(): async write failed: {}", ec.message());
            else
                g_logger->debug("http_session::start_write(): async write successful");
            self->_stream.socket().shutdown(net::socket::shutdown_send, ec); });
    }

    void handle_request()
    {
        http_result result;
        g_logger->debug("http_session::handle_request(): handling request for target {}", _request.target());
        if (_request.method() == http::verb::get)
        {
            result.emplace<distdb_resource>();
            if (distdb_find(_archive.get(), std::string(_request.target()).c_str(), &std::get<distdb_resource>(result)) != 0)
                result.emplace<http::status>(http::status::not_found);
        }
        else
        {
            result.emplace<http::status>(http::status::method_not_allowed);
        }

        if (std::holds_alternative<distdb_resource>(result))
        {
            g_logger->debug("http_session::handle_request(): resource found, preparing response");
            const distdb_resource &resource = std::get<distdb_resource>(result);
            auto response = std::make_shared<http::response<http::span_body<const uint8_t>>>(http::status::ok, _request.version());
            response->set(http::field::content_type, resource.mime_type);
            response->set(http::field::content_length, std::to_string(resource.length));
            response->body() = beast::span<const uint8_t>(reinterpret_cast<const uint8_t *>(resource.content), resource.length);
            response->prepare_payload();
            start_write(response);
        }
        else if (std::holds_alternative<http::status>(result))
        {
            g_logger->debug("http_session::handle_request(): preparing error response");
            auto response = std::make_shared<http::response<http::empty_body>>(std::get<http::status>(result), _request.version());
            response->prepare_payload();
            start_write(response);
        }
    }

public:
    static bool prepare_archive(const char *path)
    {
        g_logger->debug("http_session::prepare_archive(): opened archive at path {}", path);
        distdb_archive *archive(nullptr);
        distdb_status status = distdb_open(path, &archive);
        if (status == 0)
        {
            _archive.reset(archive);
        }
        else
        {
            g_logger->error("http_session::prepare_archive(): failed to open archive: {}", status);
        }
        return archive != nullptr;
    }

    void start()
    {
        start_read();
    }
};

class http_listener : std::enable_shared_from_this<http_listener>
{
private:
    asio::io_context &_context;
    net::acceptor _acceptor;
    void start_accept()
    {
        _acceptor.async_accept(
            asio::make_strand(_context),
            beast::bind_front_handler(&http_listener::on_accept, this));
    }
    void on_accept(const boost::system::error_code &ec, net::socket socket)
    {
        if (!ec)
            std::make_shared<http_session>(std::move(socket))->start();
    }

public:
    http_listener(asio::io_context &context, net::acceptor &&acceptor) : _context(context), _acceptor(std::move(acceptor)) {}
    ~http_listener() = default;
    void listen()
    {
        start_accept();
    }
};

class http_engine
{
private:
    asio::io_context _context;
    latch_type _latch;
    net::endpoint _endpoint;
    std::vector<std::thread> _threads;
    template <typename Func>
    bool acceptor_func(Func &&op)
    {
        boost::beast::error_code ec;
        op(ec);
        return !ec;
    }

public:
    http_engine(size_t threads, const net::endpoint &endpoint) : _context(threads),
                                                                 _latch(threads),
                                                                 _endpoint(endpoint)
    {
        _threads.reserve(threads);
    }

    void start()
    {
        g_logger->debug("http_engine::start(): starting {} threads", _threads.capacity());
        while (_threads.size() < _threads.capacity())
        {
            _threads.emplace_back([this]
                                  {
                _context.run();
                _latch.count_down(); });
        }
    }

    void stop()
    {
        g_logger->debug("http_engine::stop(): stopping context");
        _context.stop();
        g_logger->debug("http_engine::stop(): context stopped");
    }

    void wait()
    {
        _latch.wait();
        g_logger->debug("http_engine::wait(): joining worker threads");
        for (auto &t : _threads)
            if (t.joinable())
                t.join();
    }

    std::unique_ptr<http_listener> create_listener()
    {
        std::unique_ptr<http_listener> result;
        net::acceptor acceptor(asio::make_strand(_context));
        if (acceptor_func([&](boost::beast::error_code &ec)
                          { acceptor.open(_endpoint.protocol(), ec); }) &&
            acceptor_func([&](boost::beast::error_code &ec)
                          { acceptor.bind(_endpoint, ec); }) &&
            acceptor_func([&](boost::beast::error_code &ec)
                          { acceptor.listen(asio::socket_base::max_listen_connections, ec); }))
        {
            result = std::make_unique<http_listener>(_context, std::move(acceptor));
        }
        return result;
    }
};

static std::unique_ptr<http_engine> g_engine;

bool initialize(int argc, const char **argv)
{
    bool result(false);
    try
    {
        argparse::ArgumentParser parser("distdbd");
        parser.add_argument("path").required().help("Path to distdb archive file");
        parser.add_argument("-l", "--log").default_value("stderr").help("Log output destination (stderr, stdout, or file path)");
        parser.add_argument("-t", "--threads").default_value(static_cast<size_t>(std::thread::hardware_concurrency())).help("Number of HTTP worker threads");
        parser.add_argument("-d", "--debug").default_value(false).help("Enable debug mode");
        parser.parse_args(argc, argv);
    }
    catch (const std::exception &e)
    {
        if (g_logger)
            g_logger->error("initialization failed: {}", e.what());
        else
            std::cerr << "distdbd failed to initialize: " << e.what() << std::endl;
    }

    return result;
}

bool launch()
{
    std::unique_ptr<http_listener> listener;
    bool result(false);

    listener = g_engine->create_listener();

    if (listener)
    {
        try
        {
            listener->listen();
            g_engine->start();
            std::signal(SIGINT, [](int)
                        { g_engine->stop(); });
            std::signal(SIGTERM, [](int)
                        { g_engine->stop(); });
            g_engine->wait();
            result = true;
        }
        catch (const std::exception &e)
        {
            g_logger->error("{}", e.what());
        }
    }

    return result;
}

int main(int argc, const char **argv)
{
    int exit_code(1);
    if (initialize(argc, argv))
    {
        exit_code = launch() ? 0 : 1;
    }
    return exit_code;
}