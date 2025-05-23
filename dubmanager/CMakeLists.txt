project(kdevddubmanager)

include(ECMInstallIcons)

find_package(KF6 COMPONENTS CoreAddons REQUIRED)

add_subdirectory(src)
add_subdirectory(icons)

target_compile_options(kdevddubmanager PRIVATE -fexceptions)

# kdebugsettings file
if (ECM_VERSION VERSION_GREATER "6.0.0")
    install(FILES kdevddubmanager.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES kdevddubmanager.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
