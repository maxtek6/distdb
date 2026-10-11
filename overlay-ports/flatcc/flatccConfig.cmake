get_filename_component(_flatcc_prefix "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

if(NOT TARGET flatcc::flatcc)
    add_library(flatcc::flatcc STATIC IMPORTED)
    set_target_properties(flatcc::flatcc PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
        IMPORTED_LOCATION_DEBUG "${_flatcc_prefix}/debug/lib/${CMAKE_STATIC_LIBRARY_PREFIX}flatcc_d${CMAKE_STATIC_LIBRARY_SUFFIX}"
        IMPORTED_LOCATION_RELEASE "${_flatcc_prefix}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}flatcc${CMAKE_STATIC_LIBRARY_SUFFIX}"
        MAP_IMPORTED_CONFIG_MINSIZEREL Release
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        INTERFACE_INCLUDE_DIRECTORIES
            "${_flatcc_prefix}/include;${_flatcc_prefix}/include/flatcc/reflection"
    )
endif()

if(NOT TARGET flatcc::flatccrt)
    add_library(flatcc::flatccrt STATIC IMPORTED)
    set_target_properties(flatcc::flatccrt PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
        IMPORTED_LOCATION_DEBUG "${_flatcc_prefix}/debug/lib/${CMAKE_STATIC_LIBRARY_PREFIX}flatccrt_d${CMAKE_STATIC_LIBRARY_SUFFIX}"
        IMPORTED_LOCATION_RELEASE "${_flatcc_prefix}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}flatccrt${CMAKE_STATIC_LIBRARY_SUFFIX}"
        MAP_IMPORTED_CONFIG_MINSIZEREL Release
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        INTERFACE_INCLUDE_DIRECTORIES
            "${_flatcc_prefix}/include;${_flatcc_prefix}/include/flatcc/reflection"
    )
endif()

if(NOT TARGET flatcc::flatcc_cli)
    add_executable(flatcc::flatcc_cli IMPORTED)
    set_target_properties(flatcc::flatcc_cli PROPERTIES
        IMPORTED_CONFIGURATIONS "DEBUG;RELEASE"
        IMPORTED_LOCATION_DEBUG "${_flatcc_prefix}/debug/bin/flatcc_d${CMAKE_EXECUTABLE_SUFFIX}"
        IMPORTED_LOCATION_RELEASE "${_flatcc_prefix}/bin/flatcc${CMAKE_EXECUTABLE_SUFFIX}"
        MAP_IMPORTED_CONFIG_MINSIZEREL Release
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
endif()

set(FLATCC_COMPILER_EXECUTABLE "${_flatcc_prefix}/bin/flatcc${CMAKE_EXECUTABLE_SUFFIX}")
unset(_flatcc_prefix)
