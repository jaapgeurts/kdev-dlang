include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Ensure the external lib is built
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/libdparser.so
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build.sh ARGS ${CMAKE_CURRENT_BINARY_DIR}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building external libdparser.so"
)
# add_custom_target(libdparser ALL DEPENDS libdparser.so)
add_custom_target(libdparser_build
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/libdparser.so
)

# Define imported shared library target
add_library(dparserlib SHARED IMPORTED GLOBAL)
set_target_properties(dparserlib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/libdparser.so"
    IMPORTED_NO_SONAME TRUE
)

add_dependencies(dparserlib libdparser_build)

set(d_parser_lib_SRC
    debug.cpp
    parsesession.cpp
)

# add_library(kdevdparser STATIC ${d_parser_lib_SRC})
add_library(kdevdparser SHARED ${d_parser_lib_SRC})

# TODO: investigate the next line
# generate_export_header(kdevdparser EXPORT_FILE_NAME kdevdparserexport.h)
target_link_libraries(kdevdparser
    PUBLIC  dparserlib
    PRIVATE KDev::Language
)

# install(TARGETS kdevdparser DESTINATION ${KDE_INSTALL_PLUGINDIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
install(TARGETS kdevdparser DESTINATION ${CMAKE_INSTALL_LIBDIR})
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdparser.so DESTINATION ${KDE_INSTALL_PLUGINDIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libdparser.so DESTINATION lib64)
