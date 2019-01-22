
# Run Flex and generate file.cpp from file.l
function(run_flex dest source skeletonFile workdir)
    message("flex(${source}) with skeleton '${skeletonFile}' -> ${dest}")

    add_custom_command(
            OUTPUT "${dest}"
            COMMAND cmd /c "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/Flex.exe" "-o${dest}" "-S${skeletonFile}" "${source}"
            WORKING_DIRECTORY "${workdir}"
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


# Run ECC and generate file.cpp from file.y
macro(run_bison dest source tempFile extra)
    message("bison(${source}) with temp '${tempFile}' -> ${dest}")

    add_custom_command(
            OUTPUT "${dest}"
            COMMAND cmd /c "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/Bison.exe" "-o${tempFile}" "${source}" -d ${extra}
            COMMAND ${CMAKE_COMMAND} -E rename "${tempFile}" "${dest}"
            WORKING_DIRECTORY "${SE_SOURCES}"
            DEPENDS "${source}"
    )

endmacro()
