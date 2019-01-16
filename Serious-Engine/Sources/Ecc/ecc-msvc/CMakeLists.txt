cmake_minimum_required(VERSION 3.4.1)
message(STATUS "PROJECT_ROOT: ${PROJECT_ROOT}")
message(STATUS "SE_SOURCES: ${SE_SOURCES}")
message(STATUS "SE_GENERATED_DIR: ${SE_GENERATED_DIR}")

add_definitions("/W3 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo")

# Generate Parser.c
add_custom_command(
        OUTPUT "${SE_GENERATED_DIR}/Ecc/Parser.c"
        COMMAND cmd /c "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/Bison.exe" "-o${SE_GENERATED_DIR}/Ecc/Parser.c" "${SE_SOURCES}/Ecc/Parser.y" -d
        WORKING_DIRECTORY "${SE_SOURCES}"
        DEPENDS "${SE_SOURCES}/Ecc/Parser.y"
)

# Generate Parser.cpp
add_custom_command(
        OUTPUT "${SE_GENERATED_DIR}/Ecc/Parser.cpp"
        COMMAND ${CMAKE_COMMAND} -E copy "${SE_GENERATED_DIR}/Ecc/Parser.c" "${SE_GENERATED_DIR}/Ecc/Parser.cpp"
        WORKING_DIRECTORY "${SE_SOURCES}"
        DEPENDS "${SE_GENERATED_DIR}/Ecc/Parser.c"
)

# Generate Scanner.cpp
add_custom_command(
        OUTPUT "${SE_GENERATED_DIR}/Ecc/Scanner.cpp"
        COMMAND cmd /c "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/Flex.exe" "-o${SE_GENERATED_DIR}/Ecc/Scanner.cpp" "${SE_SOURCES}/Ecc/Scanner.l"
        WORKING_DIRECTORY "${SE_SOURCES}"
        DEPENDS "${SE_GENERATED_DIR}/Scanner.l"
)

set(
        ECC_SOURCES
        "${SE_SOURCES}/Ecc/Main.cpp"
        "${SE_GENERATED_DIR}/Ecc/Parser.cpp"
        "${SE_GENERATED_DIR}/Ecc/Scanner.cpp"
)

add_executable(ecc ${ECC_SOURCES})
target_include_directories(ecc PUBLIC "${SE_SOURCES}")
target_include_directories(ecc PUBLIC "${SE_SOURCES}/ecc")

add_custom_command(
        TARGET ecc POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:ecc> ${CMAKE_CURRENT_BINARY_DIR}
)
