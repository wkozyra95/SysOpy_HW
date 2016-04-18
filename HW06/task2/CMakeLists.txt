add_executable(server_t2 server.c)
add_executable(client_t2 client.c)
target_link_libraries(server_t2 rt)
target_link_libraries(client_t2 rt)

