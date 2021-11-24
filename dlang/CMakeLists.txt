project(kdevdlangplugin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(ECMAddTests)
include(GenerateExportHeader)

find_package(Qt5 REQUIRED Core Widgets Test)
find_package(KF5 REQUIRED COMPONENTS ItemModels ThreadWeaver TextEditor I18n)

find_package(KDevelop REQUIRED)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)

enable_testing()

add_subdirectory(parser)
add_subdirectory(duchain)
add_subdirectory(codecompletion)
add_subdirectory(app_templates)

kdevplatform_add_plugin(kdevdlangplugin JSON kdevdlang.json SOURCES
    kdevdlangplugin.cpp
    dlangparsejob.cpp
    dhighlighting.cpp
    ddebug.cpp
)

# fix linkage to local shared module
target_link_libraries(kdevdlangplugin
    KDev::Interfaces
    KDev::Language
    KF5::ThreadWeaver
    KF5::TextEditor
    kdevdparser
    kdevdduchain
    kdevdcompletion
    druntime-ldc-shared
#    dparser
)
