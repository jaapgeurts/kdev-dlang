set(dfmt_SRCS
    dfmt.cpp
)

ecm_qt_declare_logging_category(dfmt_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_DFMT
    CATEGORY_NAME "kdevelop.plugins.dfmt"
)

kdevplatform_add_plugin(dfmt
    JSON dfmt.json
    SOURCES ${dfmt_SRCS}
)

target_link_libraries(dfmt
    KDev::Interfaces
)
