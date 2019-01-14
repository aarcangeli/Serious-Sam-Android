cmake_minimum_required(VERSION 3.4.1)

add_library(SeriousSamNatives SHARED bindings.cpp)

target_link_libraries(SeriousSamNatives GLESv2)
