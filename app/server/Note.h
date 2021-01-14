#ifndef NOTE_H
#define NOTE_H

#include <filesystem>
#include <fstream>
#include <streambuf>
#include "Debug.h"

class Note
{
public:
    const std::string &getContent() const
    {
        return content;
    }

    void setContent(const std::string &c)
    {
        content = c;
        std::ofstream file(path, std::ios_base::out | std::ios_base::trunc);
        file << content;
    }

private:
    Note(const std::string &p) : path(p)
    {
        DEBUG << "read" << path;
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path))
        {
            std::ifstream file(path);
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            content.resize(size);
            file.seekg(0);
            file.read(&content[0], size);
            DEBUG << content;
        }
        else
            std::ofstream file(path, std::ios_base::out | std::ios_base::trunc);
    }

    void remove()
    {
        std::filesystem::remove(path);
    }

    std::string content;
    std::string path;
    friend class User;
};

#endif /* NOTE_H */
