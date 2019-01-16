set(ECC_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ecc-msvc")
set(ECC_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/build")
set(ECC_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/build/ecc.exe" CACHE STRING "Global scope" FORCE)
message("ECC_EXECUTABLE: ${ECC_EXECUTABLE}")

file(MAKE_DIRECTORY "${ECC_BINARY_DIR}")

# compile ecc with visual studio (we need an exe)
execute_process(
        COMMAND
        ${CMAKE_COMMAND}
        -G "Visual Studio 15 2017"
        "-DPROJECT_ROOT=${PROJECT_ROOT}"
        "-DSE_GENERATED_DIR=${SE_GENERATED_DIR}"
        "-DSE_SOURCES=${SE_SOURCES}"
        "-DCMAKE_BUILD_TYPE=Release"
        "${ECC_SOURCE_DIR}"

        WORKING_DIRECTORY "${ECC_BINARY_DIR}"
        OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/configure.log.txt"
        ERROR_FILE "${CMAKE_CURRENT_BINARY_DIR}/configure.err.txt"
        RESULT_VARIABLE retcode
)

if (NOT "${retcode}" STREQUAL "0")
    message(FATAL_ERROR "Cannot configure ecc compiler (returned ${retcode})\nSee ${CMAKE_CURRENT_BINARY_DIR}/configure.err.txt")
endif ()

add_custom_command(
        OUTPUT "${ECC_EXECUTABLE}"
        COMMAND ${CMAKE_COMMAND} --build .
        WORKING_DIRECTORY "${ECC_BINARY_DIR}"
        DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/ecc-msvc"
)

ADD_CUSTOM_TARGET(se-ecc DEPENDS "${ECC_EXECUTABLE}")
