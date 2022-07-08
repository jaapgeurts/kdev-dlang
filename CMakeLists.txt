cmake_minimum_required(VERSION 3.16)
project(kdevdlang)

find_package(ECM "5.14.0" REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(KDevPlatform 5.9.22 REQUIRED)

add_subdirectory(dfmt)
add_subdirectory(dlang)
add_subdirectory(dscanner)
add_subdirectory(dubmanager)
add_subdirectory(templates)
