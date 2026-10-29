add_executable(inspect ${CMAKE_CURRENT_SOURCE_DIR}/inspect.cpp)
target_link_libraries(inspect PRIVATE distdb xxHash::xxhash)

add_executable(server ${CMAKE_CURRENT_SOURCE_DIR}/server.c)
target_link_libraries(server PRIVATE distdb libevent::core libevent::extra libevent::pthreads)