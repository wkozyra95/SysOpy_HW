add_executable(async v1/main.c)
add_executable(sync v2/main.c)
add_executable(nonstop v3/main.c)
file(COPY generator.py DESTINATION .)
add_custom_command(TARGET async COMMAND python3 generator.py)
target_link_libraries(async pthread)
target_link_libraries(sync pthread)
target_link_libraries(nonstop pthread)
