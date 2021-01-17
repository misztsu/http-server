#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "HttpServer.h"
#include "Validators.h"
#include "User.h"

const std::filesystem::path indexHtml = "frontend_static/index.html";
const std::filesystem::path frontEndStatic = "frontend_static/";

UserManager userManager;
HttpServer server;

json getMessageJson(const std::string &message)
{
    return json{{"message", message}};
}

void pathUserExistsValidator(HttpRequest &request, HttpResponse &response)
{
    auto userId = request.getPathParam("userId");
    if (!userManager.hasUser(userId))
        response.setStatus(HttpResponse::Not_Found).setJsonBody(getMessageJson("User " + userId + " not found.")).send();
}

void pathUserLoggedInValidator(HttpRequest &request, HttpResponse &response)
{
    if (!request.hasCookie("token"))
        response.setStatus(HttpResponse::Unauthorized).setJsonBody(getMessageJson("Log in first")).send();
    else
    {
        auto userId = request.getPathParam("userId");
        User &user = userManager.getUser(userId);
        if (!user.equalToken(request.getCookie("token")))
            response.setStatus(HttpResponse::Unauthorized).setJsonBody(getMessageJson("User is not authorized to do this.")).send();
    }
}

void pathNoteExistsValidator(HttpRequest &request, HttpResponse &response)
{
    auto userId = request.getPathParam("userId");
    auto noteId = std::stoi(request.getPathParam("noteId"));
    if (!userManager.hasUser(userId))
        response.setStatus(HttpResponse::Not_Found).setJsonBody(getMessageJson("User " + userId + " not found.")).send();
    else
    {
        auto user = userManager.getUser(userId);
        if (!user.hasNote(noteId))
            response.setStatus(HttpResponse::Not_Found).setJsonBody(getMessageJson("Note " + std::to_string(noteId) + " not found.")).send();
    }
}

int main()
{

    server.get("/helloworld", [](auto &request, auto &response) {
        response.setStatus(200).setBody("Hello");
    });

    for (auto &path : {
             "/",
             "/login",
             "/users/{userId}",
             "/shared/{userId}/{noteId}"})
    {
        server.bindStaticFile(path, indexHtml);
    }

    server.post("/users/add",
                Validators::bodyStringNonEmptyNoWhitespaces("/userId"_json_pointer),
                Validators::bodyStringNonEmpty("/hash"_json_pointer),
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getBodyJson()["/userId"_json_pointer].get<std::string>();
                    auto hash = request.getBodyJson()["/hash"_json_pointer].get<std::string>();
                    if (userManager.hasUser(userId))
                    {
                        response.setStatus(HttpResponse::Conflict).setJsonBody(json{{"message", "User " + userId + " already exists."}});
                        return;
                    }
                    auto &user = userManager.addUser(userId, hash);
                    response.setStatus(HttpResponse::Created)
                        .setCookie("token", user.getNewToken(), user.getTokenExpirationDate())
                        .setJsonBody(json{{"userId", user.getUserId()}});
                });

    server.post("/users/login",
                Validators::bodyStringNonEmptyNoWhitespaces("/userId"_json_pointer),
                Validators::bodyStringNonEmpty("/hash"_json_pointer),
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getBodyJson()["/userId"_json_pointer].get<std::string>();
                    auto hash = request.getBodyJson()["/hash"_json_pointer].get<std::string>();
                    if (!userManager.hasUser(userId))
                    {
                        response.setStatus(HttpResponse::Not_Found).setJsonBody(json{{"message", "User " + userId + " not found."}});
                        return;
                    }
                    auto &user = userManager.getUser(userId);
                    if (!user.equalHash(hash))
                    {
                        response.setStatus(HttpResponse::Unauthorized).setJsonBody(json{{"message", "Wrong password."}});
                        return;
                    }
                    response.setStatus(HttpResponse::OK)
                        .setCookie("token", user.getNewToken(), user.getTokenExpirationDate())
                        .setJsonBody(json{{"userId", user.getUserId()}});
                });

    server.post("/users/logout",
                Validators::bodyStringNonEmptyNoWhitespaces("/userId"_json_pointer),
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getBodyJson()["/userId"_json_pointer].get<std::string>();
                    response.setStatus(HttpResponse::OK).deleteCookie("token").setJsonBody(json{{"userId", userId}});
                });

    server.get("/users/{userId}/notes",
                pathUserExistsValidator,
                pathUserLoggedInValidator,
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getPathParam("userId");
                    User &user = userManager.getUser(userId);
                    json body = json::array();
                    for (const auto &note : user.getNotes())
                        body.push_back(json{
                            {"userId", note.second.getUserId()},
                            {"noteId", note.second.getNoteId()},
                            {"content", note.second.getContent()}});
                    response.setStatus(HttpResponse::OK).setJsonBody(body);
                });

    server.get("/users/{userId}/notes/add",
                pathUserExistsValidator,
                pathUserLoggedInValidator,
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getPathParam("userId");
                    User &user = userManager.getUser(userId);
                    Note &note = user.addNote();
                    response.setStatus(HttpResponse::Created).setJsonBody(json{{"noteId", note.getNoteId()}, {"userId", user.getUserId()}});
                });

    server.get("/users/{userId}/notes/loadExamples",
                pathUserExistsValidator,
                pathUserLoggedInValidator,
                [&](auto &request, auto &response) {
                    auto userId = request.getPathParam("userId");
                    User &user = userManager.getUser(userId);
                    user.loadExampleNotes();
                    response.setStatus(HttpResponse::OK).setJsonBody(json{{"userId", userId}});
                });

    server.get("/users/{userId}/notes/{noteId}",
                Validators::pathParamInt("noteId"),
                pathNoteExistsValidator,
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getPathParam("userId");
                    auto noteId = std::stoi(request.getPathParam("noteId"));
                    User &user = userManager.getUser(userId);
                    Note &note = user.getNote(noteId);
                    response.setStatus(HttpResponse::OK)
                        .setJsonBody(json{{"userId", note.getUserId()}, {"noteId", note.getNoteId()}, {"content", note.getContent()}});
                });

    server.get("/users/{userId}/notes/{noteId}/delete",
                Validators::pathParamInt("noteId"),
                pathNoteExistsValidator,
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getPathParam("userId");
                    auto noteId = std::stoi(request.getPathParam("noteId"));
                    User &user = userManager.getUser(userId);
                    user.deleteNote(noteId);
                    response.setStatus(HttpResponse::OK).setJsonBody(json{{"noteId", noteId}});
                });

    server.post("/users/{userId}/notes/{noteId}/update",
                Validators::pathParamInt("noteId"),
                Validators::bodyStringNonEmpty("/content"_json_pointer),
                pathNoteExistsValidator,
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getPathParam("userId");
                    auto noteId = std::stoi(request.getPathParam("noteId"));
                    User &user = userManager.getUser(userId);
                    Note &note = user.getNote(noteId);
                    std::string content = request.getBodyJson()["/content"_json_pointer].get<std::string>();
                    note.setContent(content);
                    response.setStatus(HttpResponse::OK).setJsonBody(json{{"noteId", note.getNoteId()}});
                });

    server.bindStaticDirectory("/", frontEndStatic);

    // for testing frontend on development server
    // server.addAccessControllAllowOrigin("*");

    server.listen(3001);
}
