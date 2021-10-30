set(dubmanager_SRCS
    dubmanager.cpp
)

ecm_qt_declare_logging_category(dubmanager_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_DUBMANAGER
    CATEGORY_NAME "kdevelop.plugins.dubmanager"
)

kdevplatform_add_plugin(dubmanager
    JSON dubmanager.json
    SOURCES ${dubmanager_SRCS}
)

target_link_libraries(dubmanager
    KDev::Interfaces
)
