set (sdlang_SRCS
    sdlang.c)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(RAGEL 6.6 REQUIRED)

# First example - lexing decimal numbers - from the Ragel manual

RAGEL_TARGET(sdlang sdlang.rl ${CMAKE_CURRENT_BINARY_DIR}/sdlang.inl)

add_library(sdlang STATIC ${sdlang_SRCS} ${RAGEL_sdlang_OUTPUTS})
set_property(TARGET sdlang PROPERTY POSITION_INDEPENDENT_CODE ON)
