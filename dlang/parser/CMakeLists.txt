include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT libdparser.so COMMAND ./build.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(libdparser ALL DEPENDS libdparser.so)

# alternative to the above two commands
#add_custom_target(libdparser BYPRODUCTS libdparser.so ALL COMMAND ./build.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(d_parser_lib_SRC
    debug.cpp
    parsesession.cpp
)

find_library(DPARSER dparser HINTS ${CMAKE_CURRENT_SOURCE_DIR})

add_library(kdevdparser STATIC ${d_parser_lib_SRC})
#add_library(dparserlib SHARED IMPORTED)
#set_target_properties(dparserlib PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libdparser.so")
# this doesn't seem to be working

target_link_libraries(kdevdparser
PRIVATE
    KDev::Language
    ${DPARSER}
)

install(PROGRAMS ${DPARSER} DESTINATION lib64)
