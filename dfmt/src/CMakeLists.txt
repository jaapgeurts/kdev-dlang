set(kdevdfmt_SRCS
    dfmt_plugin.cpp
    dformatter.cpp
    dfmt_preferences.cpp
)

ecm_qt_declare_logging_category(kdevdfmt_SRCS
    HEADER debug.h
    IDENTIFIER DFMT
    CATEGORY_NAME "kdevelop.plugins.kdevdfmt"
)

find_package(KF6I18n REQUIRED)

set(dfmt_preferences_UI
    dfmt_preferences.ui
)
ki18n_wrap_ui(kdevdfmt_SRCS ${dfmt_preferences_UI} )

kdevplatform_add_plugin(kdevdfmt
    JSON kdevdfmt.json
    SOURCES ${kdevdfmt_SRCS}
)


target_link_libraries(kdevdfmt
#    KF5::CoreAddons
    KDev::Interfaces
    # KF6::I18n
)
