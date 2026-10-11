add_library(megamimes STATIC ${CMAKE_CURRENT_SOURCE_DIR}/MegaMimes.c)
target_include_directories(megamimes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})