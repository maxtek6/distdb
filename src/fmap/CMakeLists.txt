if(WIN32)
    set(FMAP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/fmap_win32.cpp)
else()
    set(FMAP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/fmap_posix.c)
endif()

add_library(fmap STATIC ${FMAP_SOURCE})
target_include_directories(fmap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    target_link_libraries(fmap PRIVATE mio::mio)
endif()
