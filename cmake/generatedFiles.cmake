
# Run Flex and generate file.cpp from file.l
function(run_flex dest source skeletonFile workdir)
    message("flex(${source}) with skeleton '${skeletonFile}' -> ${dest}")
if (UNIX AND NOT APPLE)
    set(source_unix "${dest}.input.l")
    set(skeleton_unix "${dest}.input.skl")
    add_custom_command(
            OUTPUT "${dest}"
            COMMAND "${CMAKE_COMMAND}" -DINPUT="${source}" -DOUTPUT="${source_unix}" -P "${PROJECT_ROOT}/cmake/normalize_eol.cmake"
            COMMAND "${CMAKE_COMMAND}" -DINPUT="${skeletonFile}" -DOUTPUT="${skeleton_unix}" -P "${PROJECT_ROOT}/cmake/normalize_eol.cmake"
            COMMAND "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/flex" "-o${dest}" "-S${skeleton_unix}" "${source_unix}"
            WORKING_DIRECTORY "${workdir}"
            DEPENDS "${source}" "${skeletonFile}"
    )
else ()
    add_custom_command(
            OUTPUT "${dest}"
            COMMAND cmd /c "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/Flex.exe" "-o${dest}" "-S${skeletonFile}" "${source}"
            WORKING_DIRECTORY "${workdir}"
            DEPENDS "${source}" "${skeletonFile}"
    )
endif()
endfunction()

# Run ECC and generate file.cpp from file.es
macro(run_ecc sources)
    foreach (arg IN ITEMS ${ARGN})
        set(source "${SE_BASE}/${arg}.es")
        set(dest "${SE_CURRENT_GENERATED_DIR}/${arg}.cpp")
        message("ecc(${source}) -> ${dest}")
        list(APPEND ${sources} ${dest})
if (UNIX AND NOT APPLE)
        set(source_unix "${SE_CURRENT_GENERATED_DIR}/${arg}.ecc_input.es")
        add_custom_command(
                OUTPUT "${dest}"
                COMMAND "${CMAKE_COMMAND}" -DINPUT="${source}" -DOUTPUT="${source_unix}" -P "${PROJECT_ROOT}/cmake/normalize_eol.cmake"
                COMMAND "${ECC_EXECUTABLE}" "-n${arg}" "-o${dest}" "${source_unix}"
                WORKING_DIRECTORY "${SE_SOURCES}"
                DEPENDS "${source}" "${ECC_EXECUTABLE}"
        )
else ()
        add_custom_command(
                OUTPUT "${dest}"
                COMMAND cmd /c "${ECC_EXECUTABLE}" "-n${arg}" "-o${dest}" "${source}"
                WORKING_DIRECTORY "${SE_SOURCES}"
                DEPENDS "${source}" "${ECC_EXECUTABLE}"
        )
endif ()
    endforeach ()
endmacro()


# Run ECC and generate file.cpp from file.y
macro(run_bison dest source tempFile extra)
    message("bison(${source}) with temp '${tempFile}' -> ${dest}")
if (UNIX AND NOT APPLE)
    set(source_unix "${dest}.input.y")
    add_custom_command(
            OUTPUT "${dest}"
            COMMAND "${CMAKE_COMMAND}" -DINPUT="${source}" -DOUTPUT="${source_unix}" -P "${PROJECT_ROOT}/cmake/normalize_eol.cmake"
            COMMAND "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/bison" "-o${tempFile}" "${source_unix}" -d ${extra}
            COMMAND ${CMAKE_COMMAND} -E rename "${tempFile}" "${dest}"
            WORKING_DIRECTORY "${SE_SOURCES}"
            DEPENDS "${source}"
    )
else ()
    add_custom_command(
            OUTPUT "${dest}"
            COMMAND cmd /c "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/Bison.exe" "-o${tempFile}" "${source}" -d ${extra}
            COMMAND ${CMAKE_COMMAND} -E rename "${tempFile}" "${dest}"
            WORKING_DIRECTORY "${SE_SOURCES}"
            DEPENDS "${source}"
    )
endif ()
endmacro()

