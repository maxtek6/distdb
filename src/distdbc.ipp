/*
 * Copyright (c) 2026 Maxtek Consulting
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
