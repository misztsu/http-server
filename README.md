# Http-server

Http-server is an asynchronous HTTP server C++ library, build using `epoll` and C++20 coroutines.
This repository also contains example note sharing app build with it.

### Code example:

```C++
#include "HttpServer.h"
#include "Validators.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;
// ...

int main()
{
    HttpServer server;

    server.post("/users/{userId}/notes/{noteId}",
            Validators::bodyStringMaxLength("/content"_json_pointer, maxNoteLength),
            // ...
            [&](HttpRequest &request, HttpResponse &response) {
                auto userId = request.getPathParam("userId");
                User &user = userManager.getUser(userId); // may throw UserNotFound
                // ...
                response.setStatus(HttpResponse::Created).setJsonBody(json{{"message", "ok"} /* ... */});
            });

    server.addExceptionHandler([](const std::exception &e, HttpResponse &response) {
        try {
            auto &unf = dynamic_cast<const UserManager::UserNotFound &>(e);
            response.setStatus(404).setJsonBody(json{{"message", unf.what()}}).send();
        } catch (const std::bad_cast &ignored) {}
    });

    auto staticPath = "static/";
    server.bindStaticDirectory("/static", staticPath);
    // ...

    int port = 3001;
    server.listen(port);
}
```

## Clone

```sh
git clone --recurse-submodules https://github.com/misztsu/http-server
```

## Building

### Build server

Build with `cmake`. This server works only on linux (because of `epoll` )
and requires `g++` version 10+ (because of coroutines).
In case it is not your default C++ compiler you can use
`DCMAKE_CXX_COMPILER` option, for example on Ubuntu 20.04:

```sh
cmake -B build -DCMAKE_CXX_COMPILER=g++-10
cmake --build build
```

### Build client

Requires `npm` and `node`.

```sh
./build_frontend.sh
```

## Run

```sh
cd bin/
./kittyNotes 3000
```

```sh
google-chrome http://localhost:3000/
```