#include <distdb.h>

#include <xxhash.h>

#include <iostream>
#include <memory>
#include <vector>


static void print_resource(const distdb_archive *archive, const char *path)
{
    distdb_resource resource;
    if (distdb_find(archive, path, &resource) == DISTDB_SUCCESS)
    {
        std::cout << path << ": " << resource.mime_type << ", " << resource.length << std::endl;
    }
    else
    {
        std::cerr << "failed to find resource " << path << std::endl;
    } 
}

static bool print_archive(const char *filepath)
{
    std::cerr << "printing " << filepath << std::endl;
    distdb_archive *archive;
    const distdb_status status = distdb_open(filepath, &archive); 
    const bool result = (status == DISTDB_SUCCESS);
    if (result)
    {
        std::cerr << "listing resources in " << filepath << std::endl;
        std::vector<const char *> paths;
        size_t count;
        distdb_list(archive, nullptr, &count);
        paths.resize(count);
        distdb_list(archive, paths.data(), &count);
        for (const auto &path : paths)
        {
            print_resource(archive, path);
        }

        distdb_close(archive);
    }
    else
    {
        std::cerr << "failed to open " << filepath << " with status " << status << std::endl;
    }
    return result;
}

int main(int argc, const char **argv)
{
    int rc(1);
    if (argc > 1)
    {
        rc = print_archive(argv[1]) ? 0 : 1;
    }
    else
    {
        std::cerr << "usage: inspect <archive path>" << std::endl;
    }
    return rc;
}