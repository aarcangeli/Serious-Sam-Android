cmake_minimum_required(VERSION 3.4.1)

get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)
add_subdirectory(${PROJECT_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/Serious-Sam-Android")

add_library(SeriousSamNatives SHARED bindings.cpp)
target_link_libraries(SeriousSamNatives GLESv2 log)

add_dependencies(SeriousSamNatives se-ecc)
