#ifndef NOTE_H
#define NOTE_H

#include <filesystem>
#include <fstream>
#include <streambuf>

#include <Debug.h>
#include <File.h>

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

    std::string getNoteId() const
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
        NoteException(const std::string &message, const std::string &userId, const std::string &noteId) : std::logic_error(message), userId(userId), noteId(noteId) {}
        const std::string &getUserId() const { return userId; }
        const std::string &getNoteId() const { return noteId; }
    private:
        const std::string userId;
        const std::string noteId;
    };

    class NoteNotFound : public NoteException
    {
    public:
        NoteNotFound(const std::string &userId, const std::string &noteId) : NoteException("Note " + userId + "/" + noteId + " not found", userId, noteId) {}
    };

    class OldVersion : public NoteException
    {
    public:
        OldVersion(std::string content, const std::string &userId, std::string noteId) :
            NoteException("Note " + userId + "/" + noteId + " has a new version", userId, noteId),
            content(content) {}
        const std::string &getContent() const { return content; }
    private:
        std::string content;
    };



private:
    Note(const std::string &userId, const std::string noteId) : userId(userId), noteId(noteId), path("users/" + userId + "/" + noteId + ".md")
    {
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
    const std::string noteId;
    const std::filesystem::path path;
    friend class User;

    mutable std::shared_mutex mutex;
};

#endif /* NOTE_H */
