add_library(kdevdduchain SHARED
	builders/declarationbuilder.cpp
	builders/contextbuilder.cpp
	builders/typebuilder.cpp
	builders/usebuilder.cpp
	builders/ddeclaration.cpp
	builders/dclassfunctiondeclaration.cpp
	builders/dintegraltype.cpp
	navigationcontext.cpp
	navigationwidget.cpp
	helper.cpp
	duchaindebug.cpp
)

generate_export_header(kdevdduchain EXPORT_FILE_NAME dduchainexport.h)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)


target_link_libraries(kdevdduchain
PRIVATE
    dparser
    kdevdparser
    KDev::Interfaces
    KDev::Language
    KDev::Shell
    KDev::Project
 )
# install(TARGETS kdevdduchain DESTINATION ${KDE_INSTALL_PLUGINDIR}/kdevplatform/${KDEV_PLUGIN_VERSION})
install(TARGETS kdevdduchain DESTINATION ${CMAKE_INSTALL_LIBDIR})
