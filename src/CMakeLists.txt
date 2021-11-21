set(dscanner_SRCS
    dscanner.cpp
)

ecm_qt_declare_logging_category(dscanner_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_DSCANNER
    CATEGORY_NAME "kdevelop.plugins.dscanner"
)

kdevplatform_add_plugin(dscanner
    JSON dscanner.json
    SOURCES ${dscanner_SRCS}
)

target_link_libraries(dscanner
    KDev::Interfaces
)
