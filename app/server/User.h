#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <mutex>
#include <string>

#include "Note.h"
#include "utils/SharedMap.h"

class User
{
public:
    using NotePtr = std::shared_ptr<Note>;

    const SharedMap<int, NotePtr> &getNotes() const
    {
        return notes;
    }

    bool hasNote(int noteId)
    {
        return notes.hasKey(noteId);
    }

    NotePtr getNote(int noteId)
    {
        try {
            return notes.at(noteId);
        } catch (const std::out_of_range &e) {
            throw Note::NoteNotFound(userId, noteId);
        }
    }

    NotePtr addNote()
    {
        maxNoteId++;
        return notes.emplace(maxNoteId, new Note(userId, maxNoteId)).first->second;
    }

    void deleteNote(int noteId, std::string oldContent)
    {
        try {
            notes.at(noteId)->remove(oldContent);
            notes.erase(noteId);
        } catch (const std::out_of_range &e) {
            throw Note::NoteNotFound(userId, noteId);
        }
    }

    void loadExampleNotes()
    {
        addNote()->setContent("Zagadka (C++):\n\n```\nint main() {\n    (+[]{})();\n}\n```\n\nDlaczego ten kod się kompiluje?", "");
        addNote()->setContent("Do zrobienia:\n\n- [x] Sieci komputerowe\n- [x] Systemy zarządzania bazami danych", "");
        addNote()->setContent("> Litwo! Ojczyzno moja! ty jesteś jak zdrowie.\n> Ile cię trzeba cenić, ten tylko się dowie,\n> Kto cię stracił. Dziś piękność twą w całej ozdobie\n> Widzę i opisuję, bo tęsknię po tobie.\n*Pan Tadeusz* Adam Mickiewicz", "");
    }

    const std::string &getUserId() const
    {
        return userId;
    }

    bool equalHash(const std::string &hashToTest) const
    {
        return hashToTest == hash;
    }

    std::string getNewToken()
    {
        start = std::chrono::system_clock::now();
        end = start + tokenMaxAge;
        std::stringstream seed;
        seed << userId << std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();
        return token = std::to_string(std::hash<std::string>{}(seed.str()));
    }

    bool equalToken(const std::string &tokenToTest) const
    {
        return end > std::chrono::system_clock::now() && token == tokenToTest;
    }

    std::string getTokenExpirationDate() const
    {
        auto end_time_t = std::chrono::system_clock::to_time_t(end);

        std::stringstream utcString;
        utcString << std::put_time(std::localtime(&end_time_t), "%a, %d %b %Y %H:%M:%S GMT");
        return utcString.str();
    }

private:
    User(const std::string &userId_) : userId(userId_)
    {
        std::filesystem::path path = "users/" + userId;
        std::filesystem::create_directory(path);
        for (auto &p : std::filesystem::directory_iterator(path))
        {
            if (p.path().filename() == "hash.txt")
            {
                // TODO set hash only once
                hash = file::readAll(p.path());
            }
            else if (p.path().extension() == ".txt")
            {
                int id = std::stoi(p.path().stem().string());
                maxNoteId = id > maxNoteId ? id : maxNoteId;
                notes.emplace(id, new Note(userId, id));
            }
        }
    }

    User(const std::string &userId_, const std::string &hash_) : User(userId_)
    {
        // TODO set hash only once
        hash = hash_;
        file::writeAll("users/" + userId + "/hash.txt", hash);
    }

    int maxNoteId = 0;
    SharedMap<int, NotePtr> notes;
    const std::string userId;
    std::string hash;

    std::string token;
    static constexpr std::chrono::minutes tokenMaxAge{60};
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point end = start + tokenMaxAge;

    friend class UserManager;
};

class UserManager
{
public:
    UserManager()
    {
        loadUsers();
    }

    User &addUser(const std::string &userId, const std::string &hash)
    {
        if (hasUser(userId))
            throw UserExists(userId);

        User &user = users.emplace(userId, User(userId, hash)).first->second;
        user.loadExampleNotes();
        return user;
    }

    User &getUser(std::string userId)
    {
        try {
            return users.at(userId);
        } catch (const std::out_of_range &e) {
            throw UserNotFound(userId);
        }
    }

    bool hasUser(std::string userId) const
    {
        return users.hasKey(userId);
    }

    class UserExists : public std::out_of_range
    {
    public:
        UserExists(const std::string &userId) : std::out_of_range("User " + userId + " already exists"), userId(userId) {}
        const std::string &getUserId() const { return userId; }
    private:
        const std::string userId;
    };

    class UserNotFound : public std::out_of_range
    {
    public:
        UserNotFound(const std::string &userId) : std::out_of_range("User " + userId + " not found"), userId(userId) {}
        const std::string &getUserId() const { return userId; }
    private:
        const std::string userId;
    };

private:
    void loadUsers()
    {
        std::filesystem::create_directory("users");
        for (auto &p : std::filesystem::directory_iterator("users"))
        {
            auto userId = p.path().filename().string();
            users.emplace(userId, User(userId));
        }
    }

    SharedMap<std::string, User> users;
};

#endif /* USER_H */
