add_subdirectory(alog)
add_subdirectory(eventx)
add_subdirectory(fmap)
add_subdirectory(megamimes)
add_subdirectory(schema)

file(GLOB DISTDB_SOURCES "distdb_*.c")
add_library(distdb STATIC ${DISTDB_SOURCES})
target_include_directories(
    distdb 
    PUBLIC ${DISTDB_INCLUDE_DIR} 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(distdb PRIVATE distdb_schema flatcc::flatccrt fmap)


target_sources(distdbc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/distdbc.cpp)
target_include_directories(distdbc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(distdbc PRIVATE distdb_schema argparse::argparse flatbuffers::flatbuffers megamimes xxHash::xxhash mio::mio)

target_sources(distdbd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/distdbd.c)
target_include_directories(distdbd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(distdbd PRIVATE distdb eventx alog argtable3::argtable3)