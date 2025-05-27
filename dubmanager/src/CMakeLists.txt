
set(kdevddubmanager_SRCS
    toolchain/dmdtoolchain.cpp
    toolchain/gdctoolchain.cpp
    toolchain/ldc2toolchain.cpp
    toolchain/toolchainwidget.cpp

    dcompilerfilterstrategy.cpp
    dubmanager.cpp
    dubsettings.cpp
    # dubconfig.cpp
    # dubparser.cpp
    dubbuilder.cpp
    dubsettingitem.cpp
    dubpreferences.cpp
    dubjob.cpp
    sdlparser.cpp
)

ecm_qt_declare_logging_category(kdevddubmanager_SRCS
    HEADER debug.h
    IDENTIFIER DUB
    CATEGORY_NAME "kdev.dlang.dubmanager"
    DEFAULT_SEVERITY Debug
)


ki18n_wrap_ui(kdevddubmanager_SRCS dubpreferences.ui )
ki18n_wrap_ui(kdevddubmanager_SRCS toolchain/toolchainwidget.ui)


kdevplatform_add_plugin(kdevddubmanager
    JSON kdevddubmanager.json
    SOURCES ${kdevddubmanager_SRCS}
)

target_link_libraries(kdevddubmanager
# Ideally this is linked to the project itself, not the file

    KDev::Interfaces
    KDev::Project
    KDev::Util

    # KURL file requestor
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::KIOWidgets
    #KDev::OutputView
)
