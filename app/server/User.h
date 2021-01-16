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
        addNote().setContent("Zagadka (C++):\n\n```\nint main() {\n    (+[]{})();\n}\n```\n\nDlaczego ten kod się kompiluje?");
        addNote().setContent("Do zrobienia:\n\n- [x] Sieci komputerowe\n- [ ] Systemy zarządzania bazami danych");
        addNote().setContent("> Litwo! Ojczyzno moja! ty jesteś jak zdrowie.\n> Ile cię trzeba cenić, ten tylko się dowie,\n> Kto cię stracił. Dziś piękność twą w całej ozdobie\n> Widzę i opisuję, bo tęsknię po tobie.\n*Pan Tadeusz* Adam Mickiewicz");
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
