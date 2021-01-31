# Http-server

Http-server is asynchronous HTTP server using `epoll` and C++20 coroutines.

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