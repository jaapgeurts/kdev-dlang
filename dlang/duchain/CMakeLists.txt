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

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

# TODO: investigate this next line
generate_export_header(kdevdduchain EXPORT_FILE_NAME dduchainexport.h)
target_link_libraries(kdevdduchain
PRIVATE
    dparser
    kdevdparser
    KDev::Interfaces
    KDev::Language
    KDev::Shell
    KDev::Project
 )
