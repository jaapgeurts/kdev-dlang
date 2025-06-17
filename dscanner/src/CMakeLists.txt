set(kdevdscanner_SRCS
    dscanner_plugin.cpp
    job.cpp
    parameters.cpp
    dproblemmodel.cpp
    utils.cpp

    config/globalconfigpage.cpp
    config/projectconfigpage.cpp
)

ecm_qt_declare_logging_category(kdevdscanner_SRCS
    HEADER debug.h
    IDENTIFIER DSCANNER
    CATEGORY_NAME "kdevelop.plugins.dscanner"
)

find_package(KF6KIO)

kconfig_add_kcfg_files(kdevdscanner_SRCS
    config/globalsettings.kcfgc
    config/projectsettings.kcfgc
)

ki18n_wrap_ui(kdevdscanner_SRCS
    config/globalconfigpage.ui
    config/projectconfigpage.ui
)
qt6_add_resources(kdevdscanner_SRCS
    kdevdscanner.qrc
)

kdevplatform_add_plugin(kdevdscanner
    JSON kdevdscanner.json
    SOURCES ${kdevdscanner_SRCS}
)
target_link_libraries(kdevdscanner
#    KDev::Interfaces
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::KIOWidgets
    KDev::Language
    KDev::Project
    KDev::Shell
)
