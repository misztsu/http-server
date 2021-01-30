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
    Note(Note &&other) : content(std::move(other.content)), userId(std::move(other.userId)), noteId(other.noteId), path(std::move(other.path)) {}
    Note(const Note &other) : userId(std::move(other.userId)), noteId(other.noteId), path(std::move(other.path))
    {
        std::shared_lock lock(other.mutex);
        content = other.content;
    }

    std::string getContent() const
    {
        std::shared_lock lock(mutex);
        checkIfDeleted();
        return content;
    }

    void setContent(const std::string &newContent, const std::string &oldContent)
    {
        std::unique_lock lock(mutex);
        checkIfDeleted();
        if (oldContent != content)
            throw OldVersion(content, userId, noteId);
        content = newContent;
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

    void remove(std::string oldContent)
    {
        std::unique_lock lock(mutex);
        checkIfDeleted();
        if (oldContent != content)
            throw OldVersion(content, userId, noteId);
        std::filesystem::remove(path);
        deleted = true;
    }

    void unconditionalRemove()
    {
        std::unique_lock lock(mutex);
        if (!deleted)
        {
            std::filesystem::remove(path);
            deleted = true;
        }
    }

    bool isDeleted() const { return deleted; }

    class NoteException : public std::logic_error
    {
    public:
        NoteException(std::string message, const std::string &userId, int noteId) : std::logic_error(message), userId(userId), noteId(noteId) {}
        const std::string &getUserId() const { return userId; }
        int getNoteId() const { return noteId; }
    private:
        const std::string userId;
        const int noteId;
    };

    class NoteNotFound : public NoteException
    {
    public:
        NoteNotFound(const std::string &userId, int noteId) : NoteException("Note " + userId + "/" + std::to_string(noteId) + " not found", userId, noteId) {}
    };

    class OldVersion : public NoteException
    {
    public:
        OldVersion(std::string content, const std::string &userId, int noteId) :
            NoteException("Note " + userId + "/" + std::to_string(noteId) + " has a new version", userId, noteId),
            content(content) {}
        const std::string &getContent() { return content; }
    private:
        std::string content;
    };

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

    void checkIfDeleted() const
    {
        if (deleted)
            throw NoteNotFound(userId, noteId);
    }

    bool deleted = false;

    std::string content;
    const std::string userId;
    const int noteId;
    const std::filesystem::path path;
    friend class User;

    mutable std::shared_mutex mutex;
};

#endif /* NOTE_H */
