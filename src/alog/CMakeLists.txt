if (WIN32)
    set(ALOG_PLATFORM_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/alog_win32.cpp)
else()
    set(ALOG_PLATFORM_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/alog_posix.c)
endif()

add_library(alog STATIC ${CMAKE_CURRENT_SOURCE_DIR}/alog.c ${ALOG_PLATFORM_SOURCE})
target_include_directories(alog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
