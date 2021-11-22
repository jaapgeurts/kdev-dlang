set(dscanner_plugin_SRCS
    dscanner_plugin.cpp
    job.cpp
    parameters.cpp
    dproblemmodel.cpp
    utils.cpp

    config/globalconfigpage.cpp
    config/projectconfigpage.cpp
)

ecm_qt_declare_logging_category(dscanner_plugin_SRCS
    HEADER debug.h
    IDENTIFIER DSCANNER
    CATEGORY_NAME "kdevelop.plugins.dscanner"
)

kconfig_add_kcfg_files(dscanner_plugin_SRCS
    config/globalsettings.kcfgc
    config/projectsettings.kcfgc
)

ki18n_wrap_ui(dscanner_plugin_SRCS
    config/globalconfigpage.ui
    config/projectconfigpage.ui
)
qt5_add_resources(dscanner_plugin_SRCS
    kdevdscanner.qrc
)

kdevplatform_add_plugin(dscanner_plugin
    JSON kdevdscanner_plugin.json
    SOURCES ${dscanner_plugin_SRCS}
)

target_link_libraries(dscanner_plugin
#    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Shell
)
