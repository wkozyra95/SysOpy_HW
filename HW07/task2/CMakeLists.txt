add_executable(reader reader.c)
add_executable(writer writer.c)
add_executable(shared_posix shared.c)

target_link_libraries(shared_posix rt pthread)
target_link_libraries(writer rt pthread)
target_link_libraries(reader rt pthread)