#ifndef FILE_H_
#define FILE_H_

#include <filesystem>
#include <fstream>
#include <streambuf>
#include "Debug.h"

namespace file
{
    inline std::string readAll(const std::string &path)
    {
        std::string content;
        std::ifstream file(path);
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        content.resize(size);
        file.seekg(0);
        file.read(&content[0], size);
        return content;
    }

    inline std::string readAll(const std::filesystem::path &path)
    {
        return readAll(path.string());
    }

    inline void writeAll(const std::string &path, const std::string &content)
    {
        std::ofstream file(path, std::ios_base::out | std::ios_base::trunc);
        file << content;
    }

    inline void writeAll(const std::filesystem::path &path, const std::string &content)
    {
        writeAll(path.string(), content);
    }

    inline void create(const std::string &path)
    {
        std::ofstream file(path, std::ios_base::out | std::ios_base::trunc);
    }

    inline void create(const std::filesystem::path &path)
    {
        return create(path.string());
    }

} // namespace file

#endif