include_guard(GLOBAL)

function(qscss_add_style_target target_name)
    set(one_value_args WORKING_DIRECTORY)
    cmake_parse_arguments(QSCSS "" "${one_value_args}" "" ${ARGN})

    if(NOT QSCSS_WORKING_DIRECTORY)
        set(QSCSS_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    find_program(QSCSS_NPM_EXECUTABLE npm)
    if(NOT QSCSS_NPM_EXECUTABLE)
        message(WARNING "npm was not found; target ${target_name} will be unavailable.")
        return()
    endif()

    add_custom_target(${target_name}
        COMMAND ${QSCSS_NPM_EXECUTABLE} run build:qss
        WORKING_DIRECTORY ${QSCSS_WORKING_DIRECTORY}
        COMMENT "Compiling SCSS theme entries to QSS"
        VERBATIM
    )
endfunction()

