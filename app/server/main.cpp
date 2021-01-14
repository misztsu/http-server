#include <string>

#include "HttpServer.h"
#include "User.h"

std::string jsonFromId(int id)
{
    std::string body = "{\n\"id\": ";
    body.append(std::to_string(id));
    body.append("\"\n}");
    return body;
}

int main()
{
    UserManager userManager;
    HttpServer server;

    server.get("/", [](auto &request, auto &response) {
        response.setStatus(200);
        response.setBody("Hello");
    });

    server.get("/users/{userId}/notes", [&](auto &request, auto &response) {
        try
        {
            int userId = std::stoi(request.getPathParam("userId"));
            User &user = userManager.getUser(userId);

            std::string body = "[\n";
            for (const auto &note : user.getNotes())
            {
                body.append("{\n\"id\": ");
                body.append(std::to_string(note.first));
                body.append(",\n\"content\": \"");
                body.append(note.second.getContent());
                body.append("\"\n},");
            }
            body.pop_back();
            body.append("\n]");

            response.setStatus(200);
            response.setBody(body, "application/json");
        }
        catch (std::invalid_argument &e)
        {
            response.setStatus(HttpResponse::Status::Bad_Request);
        }
        catch (std::exception &)
        {
            response.setStatus(HttpResponse::Status::Internal_Server_Error);
        }
    });

    server.get("/users/{userId}/notes/add", [&](auto &request, auto &response) {
        try
        {
            int userId = std::stoi(request.getPathParam("userId"));
            User &user = userManager.getUser(userId);
            auto &note = user.addNote();

            response.setStatus(201);
            response.setBody(jsonFromId(note.first), "application/json");
        }
        catch (std::invalid_argument &e)
        {
            response.setStatus(HttpResponse::Status::Bad_Request);
        }
        catch (std::exception &)
        {
            response.setStatus(HttpResponse::Status::Internal_Server_Error);
        }
    });

    server.get("/users/{userId}/notes/{noteId}/delete", [&](auto &request, auto &response) {
        try
        {
            int userId = std::stoi(request.getPathParam("userId"));
            int noteId = std::stoi(request.getPathParam("noteId"));
            User &user = userManager.getUser(userId);
            user.deleteNote(noteId);

            response.setStatus(200);
            response.setBody(jsonFromId(noteId), "application/json");
        }
        catch (std::invalid_argument &e)
        {
            response.setStatus(HttpResponse::Status::Bad_Request);
        }
        catch (std::exception &)
        {
            response.setStatus(HttpResponse::Status::Internal_Server_Error);
        }
    });

    server.get("/users/{userId}/notes/{noteId}/update/{content}", [&](auto &request, auto &response) {
        try
        {
            int userId = std::stoi(request.getPathParam("userId"));
            int noteId = std::stoi(request.getPathParam("noteId"));
            std::string content = request.getPathParam("content");
            User &user = userManager.getUser(userId);
            Note &note = user.getNote(noteId);
            note.setContent(content);

            response.setStatus(200);
            response.setBody(jsonFromId(noteId), "application/json");
        }
        catch (std::invalid_argument &e)
        {
            response.setStatus(HttpResponse::Status::Bad_Request);
        }
        catch (std::exception &)
        {
            response.setStatus(HttpResponse::Status::Internal_Server_Error);
        }
    });

    server.get("/users/{userId}/notes/loadExamples", [&](auto &request, auto &response) {
        try
        {
            int userId = std::stoi(request.getPathParam("userId"));
            User &user = userManager.getUser(userId);
            user.loadExampleNotes();

            response.setStatus(200);
            response.setBody(jsonFromId(userId), "application/json");
        }
        catch (std::invalid_argument &e)
        {
            response.setStatus(HttpResponse::Status::Bad_Request);
        }
        catch (std::exception &)
        {
            response.setStatus(HttpResponse::Status::Internal_Server_Error);
        }
    });

    server.get("/users/add", [&](auto &request, auto &response) {
        try
        {
            auto &user = userManager.addUser();

            response.setStatus(201);
            response.setBody(jsonFromId(user.first), "application/json");
        }
        catch (std::invalid_argument &e)
        {
            response.setStatus(HttpResponse::Status::Bad_Request);
        }
        catch (std::exception &)
        {
            response.setStatus(HttpResponse::Status::Internal_Server_Error);
        }
    });

    server.listen(3001);
}
