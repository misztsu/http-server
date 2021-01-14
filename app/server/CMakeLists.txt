add_executable(kittyNotes main.cpp)
target_include_directories(kittyNotes PRIVATE ../../src)
target_link_libraries(kittyNotes PRIVATE Threads::Threads)
