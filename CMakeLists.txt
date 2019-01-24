set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
SET(CMAKE_USE_RELATIVE_PATHS OFF)
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SE_SOURCES "${PROJECT_ROOT}/Serious-Engine/Sources")

add_compile_options(-Wnon-pod-varargs)
add_compile_options(-Wextern-initializer)

set(SE_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/Serious-Engine/Gen")
file(MAKE_DIRECTORY "${SE_GENERATED_DIR}")

message("PROJECT_ROOT: ${PROJECT_ROOT}")
message("SE_GENERATED_DIR: ${SE_GENERATED_DIR}")

add_subdirectory(Serious-Engine)
