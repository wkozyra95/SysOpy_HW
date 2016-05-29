add_executable(stream_server server.c)
add_executable(stream_client client.c)

target_link_libraries(stream_server pthread)
target_link_libraries(stream_client pthread)