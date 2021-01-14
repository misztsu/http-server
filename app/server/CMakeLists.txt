
add_executable(kittyNotes
    main.cpp
)
target_link_libraries(kittyNotes PRIVATE Threads::Threads)
target_link_libraries(kittyNotes PRIVATE http-server)
