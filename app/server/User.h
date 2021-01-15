#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <string>
#include "Note.h"

class User
{
public:
    const std::unordered_map<int, Note> &getNotes() const
    {
        return notes;
    }

    bool hasNote(int noteId)
    {
        return notes.find(noteId) != notes.end();
    }

    Note &getNote(int noteId)
    {
        return notes.at(noteId);
    }

    Note &addNote()
    {
        maxNoteId++;
        return notes.insert({maxNoteId, Note(userId, maxNoteId)}).first->second;
    }

    void deleteNote(int noteId)
    {
        auto it = notes.find(noteId);
        if (it != notes.end())
        {
            it->second.remove();
            notes.erase(it);
        }
        else
            throw std::out_of_range("Note \"" + std::to_string(noteId) + "\" not found");
    }

    void deleteAllNotes()
    {
        notes.clear();
        maxNoteId = 0;
    }

    void loadExampleNotes()
    {
        deleteAllNotes();
        addNote().setContent("Zagadka%20(C%2B%2B)%3A%0A%0A%60%60%60%0Aint%20main()%20%7B%0A%20%20%20%20(%2B%5B%5D%7B%7D)()%3B%0A%7D%0A%60%60%60%0A%0ADlaczego%20ten%20kod%20si%C4%99%20kompiluje%3F");
        addNote().setContent("Do%20zrobienia%3A%0A%0A-%20%5Bx%5D%20Sieci%20komputerowe%0A-%20%5B%20%5D%20Systemy%20zarz%C4%85dzania%20bazami%20danych");
        addNote().setContent("%3E%20Litwo!%20Ojczyzno%20moja!%20ty%20jeste%C5%9B%20jak%20zdrowie.%0A%3E%20Ile%20ci%C4%99%20trzeba%20ceni%C4%87%2C%20ten%20tylko%20si%C4%99%20dowie%2C%0A%3E%20Kto%20ci%C4%99%20straci%C5%82.%20Dzi%C5%9B%20pi%C4%99kno%C5%9B%C4%87%20tw%C4%85%20w%20ca%C5%82ej%20ozdobie%0A%3E%20Widz%C4%99%20i%20opisuj%C4%99%2C%20bo%20t%C4%99skni%C4%99%20po%20tobie.%0A*Pan%20Tadeusz*%20Adam%20Mickiewicz");
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
                notes.insert({id, Note(userId, id)});
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
    std::unordered_map<int, Note> notes;
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
        User &user = users.insert({userId, User(userId, hash)}).first->second;
        user.loadExampleNotes();
        return user;
    }

    User &getUser(std::string userId)
    {
        return users.at(userId);
    }

    bool hasUser(std::string userId) const
    {
        return users.find(userId) != users.end();
    }

private:
    void loadUsers()
    {
        std::filesystem::create_directory("users");
        for (auto &p : std::filesystem::directory_iterator("users"))
        {
            auto userId = p.path().filename().string();
            users.insert({userId, User(userId)});
        }
    }

    std::unordered_map<std::string, User> users;
};

#endif /* USER_H */
