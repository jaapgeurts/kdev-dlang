set(kdevdubmanager_SRCS
    dubmanager.cpp
    dubbuilder.cpp
    dubpreferences.cpp
    dubjob.cpp
)

ecm_qt_declare_logging_category(kdevdubmanager_SRCS
    HEADER debug.h
    IDENTIFIER DUB
    CATEGORY_NAME "kdev.dlang.dubmanager"
    DEFAULT_SEVERITY Debug
)

set(dubpreferences_UI
    dubpreferences.ui
)

ki18n_wrap_ui(kdevdubmanager_SRCS ${dubpreferences_UI} )


kdevplatform_add_plugin(kdevdubmanager
    JSON kdevdubmanager.json
    SOURCES ${kdevdubmanager_SRCS}
)


target_link_libraries(kdevdubmanager
    KDev::Interfaces
    KDev::Project
    KDev::Util
    #KDev::OutputView
)
