set(dubbuilder_SRCS
    dubbuilder.cpp
    dubjob.cpp
)

ecm_qt_declare_logging_category(dubbuilder_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_DUBBUILDER
    CATEGORY_NAME "kdevelop.plugins.dubbuilder"
)

kdevplatform_add_plugin(dubbuilder
    JSON dubbuilder.json
    SOURCES ${dubbuilder_SRCS}
)

target_link_libraries(dubbuilder
    KDev::Interfaces
    KDev::Project
    KDev::Util
#KDev::OutputView
)
