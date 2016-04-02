
add_library(linked_list_static STATIC linked_list.c)
install(TARGETS linked_list_static DESTINATION ${CMAKE_SOURCE_DIR}/build/lib_static/)


add_library(linked_list_shared SHARED linked_list.c)
install(TARGETS linked_list_shared LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/build/lib/)
install(FILES linked_list.h DESTINATION ${CMAKE_SOURCE_DIR}/build/include/)