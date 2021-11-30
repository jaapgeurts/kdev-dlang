set(dfmt_plugin_SRCS
    dfmt_plugin.cpp
    dformatter.cpp
    dfmt_preferences.cpp
)


ecm_qt_declare_logging_category(dfmt_plugin_SRCS
    HEADER debug.h
    IDENTIFIER DFMT
    CATEGORY_NAME "kdevelop.plugins.kdevdfmt"
)

set(dfmt_preferences_UI
    dfmt_preferences.ui
)
ki18n_wrap_ui(dfmt_plugin_SRCS ${dfmt_preferences_UI} )

kdevplatform_add_plugin(dfmt_plugin
    JSON kdevdfmt_plugin.json
    SOURCES ${dfmt_plugin_SRCS}
)


target_link_libraries(dfmt_plugin
#    KF5::CoreAddons
    KDev::Interfaces
)
