

add_executable(exec_static main.c)
#target_link_libraries(exec_static ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/liblinked_list_static.a)
target_link_libraries(exec_static linked_list_static)

add_executable(exec_share main.c)
target_link_libraries(exec_share linked_list_shared)

add_executable(exec_dynamic main_dynamic.c)
target_link_libraries(exec_dynamic dl)