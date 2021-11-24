project(dscanner)

add_subdirectory(src)

# kdebugsettings file
if (ECM_VERSION VERSION_GREATER "5.58.0")
    install(FILES dscanner.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
else()
    install(FILES dscanner.categories DESTINATION ${KDE_INSTALL_CONFDIR})
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
