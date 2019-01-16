
# Run Flex and generate file.cpp from file.l
function(run_flex dest source skeletonFile)
    get_filename_component(buildDirRelFilePath "D:/../../test/ciao" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
    message("flex(${source}) with skeleton '${skeletonFile}' -> ${dest}")

    add_custom_command(
            OUTPUT "${dest}"
            COMMAND cmd /c "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/Flex.exe" "-o${dest}" "-S${skeletonFile}" "${source}"
            WORKING_DIRECTORY "${SE_SOURCES}"
            DEPENDS "${source}" "${skeletonFile}"
    )
endfunction()

# Run ECC and generate file.cpp from file.es
macro(run_ecc)
    foreach (arg IN ITEMS ${ARGN})
        set(source "${SE_BASE}/${arg}.es")
        set(dest "${SE_CURRENT_GENERATED_DIR}/${arg}.cpp")
        message("ecc(${source}) -> ${dest}")

        add_custom_command(
                OUTPUT "${dest}"
                COMMAND cmd /c "${ECC_EXECUTABLE}" "-n${arg}" "-o${dest}" "${source}" -line
                WORKING_DIRECTORY "${SE_SOURCES}"
                DEPENDS "${source}" "${ECC_EXECUTABLE}"
        )
    endforeach ()
endmacro()
