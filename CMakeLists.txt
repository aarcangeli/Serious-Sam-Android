set(SE_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/Serious-Engine/Gen")
set(SE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Serious-Engine/Sources")
file(MAKE_DIRECTORY "${SE_GENERATED_DIR}")

message("PROJECT_ROOT: ${PROJECT_ROOT}")
message("SE_GENERATED_DIR: ${SE_GENERATED_DIR}")

add_subdirectory(Serious-Engine)
