cmake_minimum_required(VERSION 3.4.1)
project(SeriousSamAndroid)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
SET(CMAKE_USE_RELATIVE_PATHS OFF)
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SE_SOURCES "${PROJECT_ROOT}/Serious-Engine/Sources")
set(LIBS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libs)
include(compilerFlags)

# build options
option(STATIC_LINKING "Buils a single .so file" OFF)
option(ENABLE_NETWORK "Enable network multiplayer mode (beta)" ON)
option(ENABLE_SPLITSCREEN "Enable splitscreen mode (not working)" OFF)
option(DISABLE_NETSETTINGS "Will skip network settings option" OFF)

if (NOT ANDROID)
    file(READ "local.properties" local)
    string(REGEX MATCH "ndk.dir=([^\r\n]*)" _ ${local})
    set(ndk ${CMAKE_MATCH_1})
    string(REPLACE "\\\\" "/" ndk ${ndk})
    string(REPLACE "\\:" ":" ndk ${ndk})
    message("ndk: ${ndk}")
    include_directories("${ndk}/sysroot/usr/include")
endif ()

set(SE_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/Serious-Engine/Gen")
file(MAKE_DIRECTORY "${SE_GENERATED_DIR}")

message("PROJECT_ROOT: ${PROJECT_ROOT}")
message("SSA_VERSION: ${SSA_VERSION}")
message("SE_GENERATED_DIR: ${SE_GENERATED_DIR}")

add_subdirectory(Serious-Engine)
