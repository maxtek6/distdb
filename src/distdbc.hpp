#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <schema_generated.h>

#include <argparse/argparse.hpp>
#include <MegaMimes.h>
#include <mio/mmap.hpp>
#include <xxhash.h>

#include <algorithm>
#include <filesystem>
#include <set>

#include <csignal>


namespace distdbc 
{
    bool argparse(int argc, const char **argv);

    int compile();

    void cancel();
}

#endif // COMPILER_HPP