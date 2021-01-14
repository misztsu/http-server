
add_library(http-server INTERFACE)

target_include_directories(http-server INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(http-server INTERFACE Threads::Threads)
target_link_libraries(http-server INTERFACE nlohmann_json::nlohmann_json)



add_executable(minimal_example
    minimalExample.cpp
)
target_link_libraries(minimal_example PRIVATE http-server)
