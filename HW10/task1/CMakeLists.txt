add_executable(datagram_server server.c)
add_executable(datagram_client client.c)

target_link_libraries(datagram_server pthread)
target_link_libraries(datagram_client pthread)