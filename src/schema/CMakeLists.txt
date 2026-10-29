add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/schema_generated.h"
    COMMAND flatbuffers::flatc
        --cpp
        -o "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema.fbs"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/schema.fbs"
    COMMENT "Generating ${CMAKE_CURRENT_SOURCE_DIR}/schema_generated.h from schema"
)

add_custom_target(generate_archive_header ALL
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/schema_generated.h"
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/schema_reader.h"
    COMMAND flatcc::flatcc_cli
        -r -v
        -o "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema.fbs"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/schema.fbs"
    COMMENT "Generating ${CMAKE_CURRENT_SOURCE_DIR}/schema_reader.h from schema using flatcc"
)

add_custom_target(generate_reader_header ALL
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/schema_reader.h"
)


add_library(distdb_schema INTERFACE)
target_include_directories(distdb_schema INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_dependencies(distdb_schema generate_archive_header generate_reader_header)