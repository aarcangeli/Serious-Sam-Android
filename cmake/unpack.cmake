function(unpack target from to)
    get_filename_component(name ${from} NAME)
    if (NOT EXISTS ${to})
        file(MAKE_DIRECTORY ${to})
    endif ()
    if (NOT ${target}_EXTRACTED)
        message(STATUS "Extracting ${name} into ${to}")
        execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xzf ${from}
                WORKING_DIRECTORY ${to}
                RESULT_VARIABLE retcode
        )
        if (NOT "${retcode}" STREQUAL "0")
            message(FATAL_ERROR "Fatal error when extracting ${from}")
        endif ()

        set(${target}_EXTRACTED ON CACHE BOOL "")
    endif ()
endfunction()
