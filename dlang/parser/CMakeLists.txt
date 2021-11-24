include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT libdparser.so COMMAND ./build.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(libdparser ALL DEPENDS libdparser.so)

set(d_parser_lib_SRC
    parsesession.cpp
)

add_library(kdevdparser STATIC ${d_parser_lib_SRC})

target_link_libraries(kdevdparser
PRIVATE
    KDev::Language
    libdparser.so
)

install(FILES libdparser.so DESTINATION lib)
