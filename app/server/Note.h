#ifndef NOTE_H
#define NOTE_H

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
        file::writeAll(path, content);
    }
    
    const std::string &getUserId() const
    {
        return userId;
    }

    int getNoteId() const
    {
        return noteId;
    }

private:
    Note(const std::string &userId_, int noteId_) : userId(userId_), noteId(noteId_), path("users/" + userId + "/" + std::to_string(noteId) + ".txt")
    {
        DEBUG << "read" << path;
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path))
        {
            content = file::readAll(path);
            DEBUG << content;
        }
        else
            file::create(path);
    }

    void remove()
    {
        std::filesystem::remove(path);
    }

    std::string content;
    const std::string userId;
    const int noteId;
    const std::filesystem::path path;
    friend class User;
};

#endif /* NOTE_H */
