include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    # ${CMAKE_SOURCE_DIR}/duchain
)

set(completion_SRCS
    model.cpp
    worker.cpp
    context.cpp
    items/completionitem.cpp
    items/functionitem.cpp
    items/importcompletionitem.cpp
    completiondebug.cpp
)

add_library(kdevdcompletion ${completion_SRCS})

generate_export_header(kdevdcompletion EXPORT_FILE_NAME dcompletionexport.h)

target_link_libraries(kdevdcompletion
PRIVATE
    KDev::Language
    KDev::Interfaces
    KDev::Project
    dparser
    kdevdparser
    kdevdduchain
)
