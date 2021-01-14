#include <iostream>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "HttpServer.h"
#include "Validators.h"

using namespace std;

int main()
{
    HttpServer server;

    server.get("/hello", [](auto &request, auto &response) {
        response.setStatus(200).setBody("Hello");
    });

    server.get("/{number}/squared",
        Validators::pathParamInt("number"),
        [] (auto &request, auto &response) {
            int number = stoi(request.getPathParam("number"));
            response.setStatus(200);
            response.setJsonBody(json{
                {"number", number},
                {"squared", number*number}
            });
        }
    );

    server.post("/register",
        Validators::bodyEmail("/email"_json_pointer),
        // auto works, but code is not smart enough to provide suggestions afterwards
        [] (HttpRequest &request, HttpResponse &response) {
            std::cout << "Should be registering user with email " << request.getBodyJson()["email"] << std::endl;
            response.setStatus(HttpResponse::Status::Not_Implemented);
        }
    );

    server.bindStaticDirectory("/", "static");

    server.listen(3000);
}
