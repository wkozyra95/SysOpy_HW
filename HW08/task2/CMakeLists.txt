add_executable(division division.c)
add_executable(signals signals.c)
target_link_libraries(division pthread)
target_link_libraries(signals pthread)