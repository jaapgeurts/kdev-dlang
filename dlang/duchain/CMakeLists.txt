add_library(kdevdduchain STATIC
	builders/declarationbuilder.cpp
	builders/contextbuilder.cpp
	builders/typebuilder.cpp
	builders/usebuilder.cpp
	builders/ddeclaration.cpp
	builders/dclassfunctiondeclaration.cpp
	navigationcontext.cpp
	navigationwidget.cpp
	helper.cpp
	duchaindebug.cpp
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

generate_export_header(kdevdduchain EXPORT_FILE_NAME dduchainexport.h)
target_link_libraries(kdevdduchain
PRIVATE
    dparser
    kdevdparser
PUBLIC
    KDev::Interfaces
    KDev::Language
    KDev::Shell
    KDev::Project
 )
