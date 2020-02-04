# Run ECC and generate file.cpp from file.es
macro(run_ecc sources)
    foreach (arg IN ITEMS ${ARGN})
        set(source "${SE_BASE}/${arg}.es")
        set(dest "${SE_CURRENT_GENERATED_DIR}/${arg}.cpp")
        message("ecc(${source}) -> ${dest}")
        list(APPEND ${sources} ${dest})

        add_custom_command(
                OUTPUT "${dest}"
                COMMAND cmd /c "${ECC_EXECUTABLE}" "-n${arg}" "-o${dest}" "${source}"
                WORKING_DIRECTORY "${SE_SOURCES}"
                DEPENDS "${source}" "${ECC_EXECUTABLE}"
        )
    endforeach ()
endmacro()