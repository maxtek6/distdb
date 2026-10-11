#ifndef COMPILER_IPP
#define COMPILER_IPP

#include <distdbc.hpp>

#ifdef _WIN32
#include <windows.h>
using fd_t = HANDLE;
#else
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
using fd_t = int;
#endif

namespace distdbc
{
    class File
    {
    private:
        fd_t fd = -1;
    public:
        struct Flags
        {
            bool read;
            bool write;
            bool create;
            bool truncate;
        };
        File() = default;
        ~File()
        {
            close();
        }
        bool open(const std::filesystem::path& path, const Flags& flags);
        void close();
        bool write(const uint8_t* data, uint64_t size);
        bool copy(const File& src, uint64_t size);
    };
}

#ifdef _WIN32

#else
bool distdbc::File::open(const std::filesystem::path& path, const Flags& flags)
{
    int openFlags = 0;
    if (flags.read && flags.write)
        openFlags |= O_RDWR;
    else if (flags.read)
        openFlags |= O_RDONLY;
    else if (flags.write)
        openFlags |= O_WRONLY;
    if (flags.create)
        openFlags |= O_CREAT;
    if (flags.truncate)
        openFlags |= O_TRUNC;

    fd = ::open(path.c_str(), openFlags, 0644);
    return fd != -1;
}

void distdbc::File::close()
{
    if (fd != -1)
    {
        ::close(fd);
        fd = -1;
    }
}

bool distdbc::File::write(const uint8_t* data, uint64_t size)
{
    if (fd == -1)
        return false;
    ssize_t written = ::write(fd, data, size);
    return written == static_cast<ssize_t>(size);
}

bool distdbc::File::copy(const File& src, uint64_t size)
{
    return sendfile(fd, src.fd, nullptr, size) == static_cast<ssize_t>(size);
}
#endif

#endif
