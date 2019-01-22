cmake_minimum_required(VERSION 3.4.1)

add_compile_options(-Wnon-pod-varargs)
add_compile_options(-Wextern-initializer)
SET(CMAKE_USE_RELATIVE_PATHS OFF)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
set(SE_SOURCES "${PROJECT_ROOT}/Serious-Engine/Sources")

add_subdirectory(${PROJECT_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/Serious-Sam-Android")
add_subdirectory(android-adapter)
add_subdirectory(bindings)
