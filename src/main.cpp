#include <string>

#include "HttpServer.h"

using namespace std;

int main()
{
    HttpServer server;

    server.get("/", [](auto &request, auto &response) {
        response.setStatus(200);
        response.setBody("Hello");
    });

    server.get("/{number}/squared", [](auto &request, auto &response) {
        try {
            int number = stoi(request.getPathParam("number"));
            response.setStatus(200);
            response.setBody(to_string(number*number), "text/plain");
        } catch (std::invalid_argument &e) {
            // TODO nice input validators
            response.setStatus(HttpResponse::Status::Bad_Request);
        }
    });

    server.listen(3000);
}
