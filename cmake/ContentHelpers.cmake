# Create cmake content project
function(CreateContent srcDir binDir)
    # Build argument list
    set(ARG_LIST -DCMAKE_GENERATOR=${CMAKE_GENERATOR})
    # Append all additional arguments
    if(${ARGC} GREATER 2)
        foreach(curr_arg ${ARGN})
            set(ARG_LIST ${ARG_LIST} -D${curr_arg})
        endforeach()
    endif()
    # Some info output
    message(STATUS "Parsed args: ${ARG_LIST}")
    # Create project
    execute_process(COMMAND ${CMAKE_COMMAND}
                    -S ${srcDir}
                    -B ${binDir}
                    ${ARG_LIST}
                    WORKING_DIRECTORY ${binDir}
                    COMMAND_ECHO STDOUT
    )
endfunction()

# Build cmake content project
function(BuildContent binDir cfgType)
    execute_process(COMMAND ${CMAKE_COMMAND}
                    --build ${binDir}
                    --config ${cfgType}
                    WORKING_DIRECTORY ${binDir}
                    COMMAND_ECHO STDOUT
    )
endfunction()

# Install cmake content project
function(InstallContent binDir cfgType installDir)
    execute_process(COMMAND ${CMAKE_COMMAND}
                    --install ${binDir}
                    --config ${cfgType}
                    --prefix ${installDir}
                    WORKING_DIRECTORY ${binDir}
                    COMMAND_ECHO STDOUT
    )
endfunction()