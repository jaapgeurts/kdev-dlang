project(kdevdubbuilder)

include(ECMInstallIcons)

find_package(KF5 COMPONENTS CoreAddons REQUIRED)

add_subdirectory(src)
add_subdirectory(icons)

# kdebugsettings file
if (ECM_VERSION VERSION_GREATER "5.58.0")
    install(FILES kdevdubbuilder.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES kdevdubbuilder.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
