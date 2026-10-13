#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include <distdb.h>

#include <boost/asio.hpp>
#include <boost/thread/latch.hpp>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

static std::unique_ptr<boost::asio::io_context> context;

static void signal_handler(int signum)
{

}

int main(int argc, const char **argv)
{
    
    return 0;
}