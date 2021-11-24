set(kdevdubbuilder_SRCS
    dubbuilder.cpp
    dubjob.cpp
)

ecm_qt_declare_logging_category(kdevdubbuilder_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_KDEVDUBBUILDER
    CATEGORY_NAME "kdevelop.plugins.kdevdubbuilder"
)

kdevplatform_add_plugin(kdevdubbuilder
    JSON kdevdubbuilder.json
    SOURCES ${kdevdubbuilder_SRCS}
)

target_link_libraries(kdevdubbuilder
    KDev::Interfaces
    KDev::Project
    KDev::Util
#KDev::OutputView
)
