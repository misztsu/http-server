#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "HttpServer.h"
#include "Validators.h"
#include "User.h"

json jsonFromId(int id)
{
    return {"id", id};
}

std::filesystem::path indexHtml = "frontend_static/index.html";
std::filesystem::path frontEndStatic = "frontend_static/";

int main()
{
    UserManager userManager;
    HttpServer server;

    server.get("/helloworld", [](auto &request, auto &response) {
        response.setStatus(200).setBody("Hello");
    });

    for (auto &path : {
        "/",
        "/users/{userId}"
    }) {
        server.bindStaticFile(path, indexHtml);
    }

    server.get("/users/{userId}/notes",
        Validators::pathParamInt("userId"),
        [&](HttpRequest &request, HttpResponse &response) {
            int userId = std::stoi(request.getPathParam("userId"));
            User &user = userManager.getUser(userId);

            json body = json::array();
            for (const auto &note : user.getNotes())
                body.push_back(json{
                    {"id", note.first},
                    {"content", note.second.getContent()}
                });

            response.setStatus(200).setJsonBody(body);
        }
    );

    server.get("/users/{userId}/notes/add",
        Validators::pathParamInt("userId"),
        [&] (auto &request, auto &response) {
            int userId = std::stoi(request.getPathParam("userId"));
            User &user = userManager.getUser(userId);
            auto &note = user.addNote();

            response.setStatus(201).setJsonBody(jsonFromId(note.first));
        }
    );

    server.get("/users/{userId}/notes/{noteId}/delete",
        Validators::pathParamInt("userId"),
        Validators::pathParamInt("noteId"),
        [&] (auto &request, auto &response) {
            int userId = std::stoi(request.getPathParam("userId"));
            int noteId = std::stoi(request.getPathParam("noteId"));
            User &user = userManager.getUser(userId);
            user.deleteNote(noteId);

            response.setStatus(200).setJsonBody(jsonFromId(noteId));
        }
    );

    // Uhhhh OHHHHH hmmmmmmmmmmmmmmmmmmmmmmmm
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
            response.setJsonBody(jsonFromId(noteId));
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

    server.get("/users/{userId}/notes/loadExamples",
        Validators::pathParamInt("userId"),
        [&] (auto &request, auto &response) {
            int userId = std::stoi(request.getPathParam("userId"));
            User &user = userManager.getUser(userId);
            user.loadExampleNotes();

            response.setStatus(200);
            response.setJsonBody(jsonFromId(userId));
        }
    );

    server.get("/users/add", [&](auto &request, auto &response) {
        auto &user = userManager.addUser();

        response.setStatus(201);
        response.setJsonBody(jsonFromId(user.first));
    });

    server.bindStaticDirectory("/", frontEndStatic);

    server.listen(3000);
}
