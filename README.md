# Http-server

Http-server jest asynchronicznym serwerem HTTP opartym o `epoll` oraz korutyny ze standardu C++20.

## Building

Build with cmake.

This version works only on linux (because of epoll)
and requires g++ version 10+ (because of coroutines).
In case it is not your default C++ compiler you can use
`DCMAKE_CXX_COMPILER` option, for example on Ubuntu 20.04:

```sh
cmake -B build -DCMAKE_CXX_COMPILER=g++-10
cmake --build build
```

## TODO:

- [x] readme
- [x] implementacja `TcpServer` i `TcpClient`
- [x] implementacja klas obietnic potrzebnych do korutyn
- [x] implementacja systemu wznawiającego korutyny z `epoll`
- [ ] implementacja `HttpServer` z jakimś interfejsem typu `server.use<GET>(std::regex(...), [](Request &req, Response &Res) { ... });`
    - [ ] połączenie z `epoll`
    - [ ] parsowanie wiadomości HTTP
    - [ ] implementacja tego `use<>`
    - [ ] `Request` i `Response`
    - [ ] cache dla zasobów
- [ ] implementacja aplikacji opartej o `HttpServer`
- [ ] frontend