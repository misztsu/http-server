
cmake_minimum_required(VERSION 3.16.3)

project(http-server)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(thirdparty/json)

# gcc 10+ only
# TODO other compilers?
add_compile_options(-Wall -Wextra -pedantic -Wno-unused-parameter)
add_compile_options(-fcoroutines)

add_subdirectory(http-server)
add_subdirectory(app/server)