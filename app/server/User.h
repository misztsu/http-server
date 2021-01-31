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

    User() = delete;
    User(User&& other)
          : notes(std::move(other.notes)),
            userId(std::move(other.userId)),
            hash(std::move(other.hash)),
            token(std::move(other.token)),
            start(other.start),
            end(other.end),
            mtRand(other.mtRand()) {}

    const SharedMap<std::string, NotePtr> &getNotes() const
    {
        return notes;
    }

    bool hasNote(const std::string &noteId)
    {
        return notes.hasKey(noteId);
    }

    NotePtr getNote(const std::string &noteId)
    {
        try {
            return notes.at(noteId);
        } catch (const std::out_of_range &e) {
            throw Note::NoteNotFound(userId, noteId);
        }
    }

    NotePtr addNote()
    {
        auto p = createNote();
        notes.emplace(p);
        return p.second;
    }

    void deleteNote(const std::string &noteId, std::string oldContent)
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
        for (auto &content : {
                "Zagadka (C++):\n\n```\nint main() {\n    (+[]{})();\n}\n```\n\nDlaczego ten kod się kompiluje?",
                "Do zrobienia:\n\n- [x] Sieci komputerowe\n- [x] Systemy zarządzania bazami danych",
                "> Litwo! Ojczyzno moja! ty jesteś jak zdrowie.\n> Ile cię trzeba cenić, ten tylko się dowie,\n> Kto cię stracił. Dziś piękność twą w całej ozdobie\n> Widzę i opisuję, bo tęsknię po tobie.\n*Pan Tadeusz* Adam Mickiewicz"
                }){   
            auto p = createNote();
            p.second->setContent(content, "");
            notes.emplace(p);
        }
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
        std::unique_lock lock(tokenMutex);
        start = std::chrono::system_clock::now();
        end = start + tokenMaxAge;
        std::stringstream seed;
        seed << userId << std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();
        return token = std::to_string(std::hash<std::string>{}(seed.str()));
    }

    bool equalToken(const std::string &tokenToTest) const
    {
        std::shared_lock lock(tokenMutex);
        return end > std::chrono::system_clock::now() && token == tokenToTest;
    }

    std::string getTokenExpirationDate() const
    {
        std::shared_lock lock(tokenMutex);
        auto end_time_t = std::chrono::system_clock::to_time_t(end);

        std::stringstream utcString;
        utcString << std::put_time(std::localtime(&end_time_t), "%a, %d %b %Y %H:%M:%S GMT");
        return utcString.str();
    }

private:
    using ull = unsigned long long;

    User(const std::string &userId, ull seed) : userId(userId), mtRand(seed)
    {
        std::filesystem::path path = "users/" + userId;
        std::filesystem::create_directory(path);
        for (auto &p : std::filesystem::directory_iterator(path))
        {
            if (p.path().filename() == "hash.txt")
            {
                hash = file::readAll(p.path());
            }
            else if (p.path().extension() == ".md")
            {
                std::string id = p.path().stem().string();
                notes.emplace(id, new Note(userId, id));
            }
        }
    }

    User(const std::string &userId_, const std::string &hash_, ull seed) : User(userId_, seed)
    {
        // TODO set hash only once
        hash = hash_;
        file::writeAll("users/" + userId + "/hash.txt", hash);
    }

    std::pair<std::string, std::shared_ptr<Note>> createNote()
    {
        std::string id = getNewId();
        return std::make_pair(id, std::shared_ptr<Note>(new Note(userId, id)));
    }

    std::string getNewId()
    {
        ull time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::stringstream s;
        s << std::hex << std::setfill('0') << std::setw(16) << time;

        std::unique_lock lock(randMutex);
        s << std::hex << std::setfill('0') << std::setw(16) << mtRand();
        s << std::hex << std::setfill('0') << std::setw(16) << mtRand();
        lock.unlock();

        return s.str();
    }

    SharedMap<std::string, NotePtr> notes;
    const std::string userId;
    std::string hash;

    std::string token;
    static constexpr std::chrono::minutes tokenMaxAge{60};
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point end = start + tokenMaxAge;

    std::mt19937_64 mtRand;
    mutable std::mutex randMutex;
    mutable std::shared_mutex tokenMutex;

    friend class UserManager;
};

class UserManager
{
public:
    UserManager()
    {
        std::random_device device;
        mtRand.seed((static_cast<unsigned long long>(device()) << 32) + device());
        loadUsers();
    }

    // TODO
    UserManager(const UserManager&) = delete;
    UserManager(UserManager&&) = delete;

    User &addUser(const std::string &userId, const std::string &hash)
    {
        std::unique_lock lock(mutex);
        auto p = users.emplace(userId, User(userId, hash, mtRand()));
        if (!p.second)
            throw UserExists(userId);
        p.first->second.loadExampleNotes();
        return p.first->second;
    }

    User &getUser(std::string userId)
    {
        std::shared_lock lock(mutex);
        try {
            return users.at(userId);
        } catch (const std::out_of_range &e) {
            throw UserNotFound(userId);
        }
    }

    bool hasUser(std::string userId) const
    {
        std::shared_lock lock(mutex);
        return users.count(userId) > 0;
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

    // no need to acquire mutex
    // because this function is called only in constructors
    void loadUsers()
    {
        std::filesystem::create_directory("users");
        for (auto &p : std::filesystem::directory_iterator("users"))
        {
            auto userId = p.path().filename().string();
            users.emplace(userId, User(userId, mtRand()));
        }
    }

    std::map<std::string, User> users;
    std::mt19937_64 mtRand;
    mutable std::shared_mutex mutex;
};

#endif /* USER_H */
