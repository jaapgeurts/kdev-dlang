set(dscanner_plugin_SRCS
    dscanner_plugin.cpp
    job.cpp
    parameters.cpp
    parser.cpp
    dproblemmodel.cpp
    utils.cpp
)

ecm_qt_declare_logging_category(dscanner_plugin_SRCS
    HEADER debug.h
    IDENTIFIER DSCANNER
    CATEGORY_NAME "kdevelop.plugins.dscanner"
)

kdevplatform_add_plugin(dscanner_plugin
    JSON kdevdscanner_plugin.json
    SOURCES ${dscanner_plugin_SRCS}
)

target_link_libraries(dscanner_plugin
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Shell
)
