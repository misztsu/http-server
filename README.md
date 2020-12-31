# Http-server

Http-server jest asynchronicznym serwerem HTTP opartym o `epoll` oraz korutyny ze standardu C++20.

## Kompilacja

_Wymagane co najmniej g++ w wersji 10._

```sh
g++ -O2 -Wall -Wextra -pedantic -std=c++20 -fcoroutines main.cpp -o server
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