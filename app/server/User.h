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

    Note &getNote(int noteId)
    {
        return notes.at(noteId);
    }

    std::pair<const int, Note> &addNote()
    {
        maxNoteId++;
        return *notes.insert({maxNoteId, Note(path + std::to_string(maxNoteId) + ".txt")}).first;
    }

    void deleteNote(int noteId)
    {
        auto it = notes.find(noteId);
        it->second.remove();
        notes.erase(it);
    }

    void deleteAllNotes()
    {
        notes.clear();
        maxNoteId = 0;
    }

    void loadExampleNotes()
    {
        deleteAllNotes();
        addNote().second.setContent("Zagadka%20(C%2B%2B)%3A%0A%0A%60%60%60%0Aint%20main()%20%7B%0A%20%20%20%20(%2B%5B%5D%7B%7D)()%3B%0A%7D%0A%60%60%60%0A%0ADlaczego%20ten%20kod%20si%C4%99%20kompiluje%3F");
        addNote().second.setContent("Do%20zrobienia%3A%0A%0A-%20%5Bx%5D%20Sieci%20komputerowe%0A-%20%5B%20%5D%20Systemy%20zarz%C4%85dzania%20bazami%20danych");
        addNote().second.setContent("%3E%20Litwo!%20Ojczyzno%20moja!%20ty%20jeste%C5%9B%20jak%20zdrowie.%0A%3E%20Ile%20ci%C4%99%20trzeba%20ceni%C4%87%2C%20ten%20tylko%20si%C4%99%20dowie%2C%0A%3E%20Kto%20ci%C4%99%20straci%C5%82.%20Dzi%C5%9B%20pi%C4%99kno%C5%9B%C4%87%20tw%C4%85%20w%20ca%C5%82ej%20ozdobie%0A%3E%20Widz%C4%99%20i%20opisuj%C4%99%2C%20bo%20t%C4%99skni%C4%99%20po%20tobie.%0A*Pan%20Tadeusz*%20Adam%20Mickiewicz");
    }

private:
    User(const std::string &userId)
    {
        path = "users/" + userId + "/";
        std::filesystem::create_directory(path);
        for (auto &p : std::filesystem::directory_iterator(path))
        {
            int id = std::stoi(p.path().stem().string());
            maxNoteId = id > maxNoteId ? id : maxNoteId;
            notes.insert({id, Note(p.path().string())});
        }
    }

    int maxNoteId = 0;
    std::unordered_map<int, Note> notes;
    std::string path;
    friend class UserManager;
};

class UserManager
{
public:
    UserManager()
    {
        loadUsers();
    }

    std::pair<const int, User> &addUser()
    {
        maxUserId++;
        auto &user = *users.insert({maxUserId, User(std::to_string(maxUserId))}).first;
        user.second.loadExampleNotes();
        return user;
    }

    User &getUser(int userId)
    {
        return users.at(userId);
    }

private:
    void loadUsers()
    {
        std::filesystem::create_directory("users");
        for (auto &p : std::filesystem::directory_iterator("users"))
        {
            auto name = p.path().filename().string();
            int id = std::stoi(name);
            maxUserId = id > maxUserId ? id : maxUserId;
            users.insert({id, User(name)});
        }
    }

    int maxUserId = 0;
    std::unordered_map<int, User> users;
};

#endif /* USER_H */
