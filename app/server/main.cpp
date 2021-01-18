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
                pathUserLoggedInValidator,
                [&](HttpRequest &request, HttpResponse &response) {
                    auto userId = request.getPathParam("userId");
                    User &user = userManager.getUser(userId);
                    Note &note = user.addNote();
                    response.setStatus(HttpResponse::Created).setJsonBody(json{{"noteId", note.getNoteId()}, {"userId", user.getUserId()}});
                });

    server.get("/users/{userId}/notes/loadExamples",
                pathUserLoggedInValidator,
                [&](auto &request, auto &response) {
                    auto userId = request.getPathParam("userId");
                    User &user = userManager.getUser(userId);
                    user.loadExampleNotes();
                    response.setStatus(HttpResponse::OK).setJsonBody(json{{"userId", userId}});
                });

    server.get("/users/{userId}/notes/{noteId}",
                Validators::pathParamInt("noteId"),
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

    server.addExceptionHandler([](const std::exception &e, HttpResponse &response) {
        try {
            auto ue = dynamic_cast<const UserManager::UserExists &>(e);
            response.setStatus(HttpResponse::Conflict).setJsonBody(getMessageJson(ue.what())).send();
        } catch (const std::bad_cast &ignored) {}
    });

    server.addExceptionHandler([](const std::exception &e, HttpResponse &response) {
        try {
            auto unf = dynamic_cast<const UserManager::UserNotFound &>(e);
            response.setStatus(404).setJsonBody(getMessageJson(unf.what())).send();
        } catch (const std::bad_cast &ignored) {}
    });

    server.addExceptionHandler([](const std::exception &e, HttpResponse &response) {
        try {
            auto nnf = dynamic_cast<const User::NoteNotFound &>(e);
            response.setStatus(404).setJsonBody(getMessageJson(nnf.what())).send();
        } catch (const std::bad_cast &ignored) {}
    });

    // for testing frontend on development server
    // server.addAccessControllAllowOrigin("*");

    server.listen(3001);
}
