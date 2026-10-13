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

#include <distdbc.ipp>

namespace distdbc
{
    struct Resource
    {
        static std::string Path(const std::filesystem::path &inputDirectory, const std::filesystem::path &inputFile)
        {
            std::string result(std::filesystem::relative(inputFile, inputDirectory).generic_string());
            if (result.front() != '/')
                result.insert(result.begin(), '/');
            return result;
        }
        static std::string Type(const std::filesystem::path &inputFile)
        {
            const std::string extension(inputFile.extension().string());
            const char *mimeType = getMegaMimeType(extension.c_str());
            std::string result("application/octet-stream");
            if (mimeType)
                result = mimeType;
            return result;
        }
        static uint64_t Hash(const std::filesystem::path &inputFile, uint64_t size, bool &ok)
        {
            uint64_t result = 0;
            if (size > 0)
            {
                mio::basic_mmap_source<uint8_t> mmap(inputFile.string());
                if (mmap.is_mapped())
                {
                    result = XXH3_64bits(mmap.data(), size);
                }
                else
                {
                    ok = false;
                }
            }
            return result;
        }
        using List = std::vector<Resource>;
        std::filesystem::path inputFile;
        std::string path;
        std::string type;
        uint64_t offset;
        uint64_t length;
        uint64_t hash;
        
        bool match(const distdb::Archive *archive, const uint8_t *data) const
        {
            const distdb::Resource *resource = archive->resources()->LookupByKey(path.c_str());
            bool result(false);
            if (resource)
            {
                if (offset == resource->offset() && length == resource->length())
                {
                    result = (length == 0) || (hash == XXH3_64bits(data + offset, length));
                }
            }
            return result;
        }

        flatbuffers::Offset<distdb::Resource> serialize(flatbuffers::FlatBufferBuilder &builder) const
        {
            return distdb::CreateResource(
                builder,
                builder.CreateString(path),
                builder.CreateString(type),
                offset,
                length
            );
        }

        bool operator<(const Resource &other) const
        {
            return path < other.path;
        }
    };

    static bool scanInputDirectory(const std::filesystem::path &inputDirectory, Resource::List &resources)
    {
        bool result(true);
        
        for (const auto &entry : std::filesystem::recursive_directory_iterator(inputDirectory))
        {
            if (result && entry.is_regular_file())
            {
                const std::filesystem::path &inputFile = entry.path();
                const uint64_t fileSize = std::filesystem::file_size(inputFile);
                Resource resource;
                resource.inputFile = inputFile;
                resource.path = Resource::Path(inputDirectory, inputFile);
                resource.type = Resource::Type(inputFile);
                resource.offset = 0;
                resource.length = fileSize;
                resource.hash = Resource::Hash(inputFile, fileSize, result);
                resources.push_back(resource);
            }
        }

        if (result)
        {
            std::sort(resources.begin(), resources.end());

            uint64_t offset = 0;
            for (Resource &resource : resources)
            {
                resource.offset = offset;
                offset += resource.length;
            }
        }

        return result;
    }

    static bool compareExistingArchive(const std::filesystem::path &archiveFile, const Resource::List &resources)
    {
        bool result(false);
        mio::basic_mmap_source<uint8_t> mmap(archiveFile.string());
        if (mmap.is_mapped())
        {
            flatbuffers::VerifierTemplate<true> verifier(mmap.data(), mmap.size());
            if (distdb::VerifySizePrefixedArchiveBuffer(verifier))
            {
                const distdb::Archive *archive = distdb::GetSizePrefixedArchive(mmap.data());
                const uint8_t *data = mmap.data() + verifier.GetComputedSize();
                if (archive->resources()->size() == resources.size())
                {
                    result = std::all_of(
                        resources.begin(),
                        resources.end(),
                        [&](const Resource &resource) {
                            return resource.match(archive, data);
                        }
                    );
                }
            }
        }
        return result;
    }

    static bool buildArchive(const std::filesystem::path &outputFile, const Resource::List &resources)
    {
        bool result(false);
        File output;
        
        if (output.open(outputFile, {false, true, true, true}))
        {            
            flatbuffers::FlatBufferBuilder builder;
            std::vector<flatbuffers::Offset<distdb::Resource>> resourceOffsets;
            bool result(false);

            for (const Resource &resource : resources)
            {
                resourceOffsets.push_back(resource.serialize(builder));
            }

            auto archiveOffset = distdb::CreateArchive(builder, builder.CreateVector(resourceOffsets));
            distdb::FinishSizePrefixedArchiveBuffer(builder, archiveOffset);
            if(output.write(builder.GetBufferPointer(), builder.GetSize()))
            {
                result = std::all_of(
                    resources.begin(),
                    resources.end(),
                    [&](const Resource &resource) {
                        File source;
                        bool copied(false);
                        if (source.open(resource.inputFile, {true, false, false, false}))
                        {
                            copied = output.copy(source, resource.length);
                            if (!copied)
                            {
                                std::cerr << "failed to copy from source file " << resource.inputFile << " to archive header " << outputFile << std::endl;
                            }
                        }
                        else
                        {
                            std::cerr << "failed to open source file " << resource.inputFile << std::endl;
                        }
                        return copied;
                    }
                );
            }
            else
            {
                std::cerr << "failed to write to archive header " << outputFile << std::endl;
            }
        }
        else
        {
            std::cerr << "failed to open file " << outputFile << std::endl;
        }
        return result;
    }
}

static std::filesystem::path _inputDirectory;
static std::filesystem::path _outputFile;
static bool _outputFileExists;

bool distdbc::argparse(int argc, const char **argv)
{
    argparse::ArgumentParser program("distdbc");
    program.add_argument("input").help("Input directory");
    program.add_argument("-o","--output").required().help("Output file");
    
    bool result(false);
    try
    {
        program.parse_args(argc, argv);
        _inputDirectory = program.get<std::string>("input");
        _outputFile = program.get<std::string>("--output");
        _outputFileExists = std::filesystem::exists(_outputFile);
        if (_outputFileExists)
        {
            if (!std::filesystem::is_regular_file(_outputFile))
            {
                throw std::runtime_error("Invalid output file");
            }
        }
        result = true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return result;
}

int distdbc::compile()
{
    distdbc::Resource::List resources;
    bool result = distdbc::scanInputDirectory(_inputDirectory, resources);
    if (result)
    {
        if (!_outputFileExists || !distdbc::compareExistingArchive(_outputFile, resources))
        {
            result = distdbc::buildArchive(_outputFile, resources);
        }
    }
    return result ? 0 : 1;
}

void distdbc::cancel()
{
    // no-op until multithreading is implemented
}

static void signal_handler(int signum)
{
    distdbc::cancel();
}

int main(int argc, const char **argv)
{
    int rc(1);
    if (distdbc::argparse(argc, argv))
    {
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        rc = distdbc::compile();
    }
    return rc;
}