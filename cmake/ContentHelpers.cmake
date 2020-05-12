# Create cmake content project
function(CreateContent SRC_DIR BIN_DIR)
    # Build argument list
    set(ARG_LIST -DCMAKE_GENERATOR=${CMAKE_GENERATOR})
    # Append all additional arguments
    if(${ARGC} GREATER 2)
        foreach(CURR_ARG ${ARGN})
            set(ARG_LIST ${ARG_LIST} -D${CURR_ARG})
        endforeach()
    endif()
    # Some info output
    message(STATUS "Parsed args: ${ARG_LIST}")
    # Create project
    execute_process(COMMAND ${CMAKE_COMMAND}
                    -S ${SRC_DIR}
                    -B ${BIN_DIR}
                    ${ARG_LIST}
                    WORKING_DIRECTORY ${BIN_DIR}
                    COMMAND_ECHO STDOUT
    )
endfunction()

# Build cmake content project
function(BuildContent BIN_DIR CONFIG)
    execute_process(COMMAND ${CMAKE_COMMAND}
                    --build ${BIN_DIR}
                    --config ${CONFIG}
                    WORKING_DIRECTORY ${BIN_DIR}
                    COMMAND_ECHO STDOUT
    )
endfunction()

# Install cmake content project
function(InstallContent BIN_DIR CONFIG INSTALL_DIR)
    # If components given
    if(${ARGC} GREATER 3)
        # Install each component
        foreach(CURR_COMP ${ARGN})
            # Some info output
            message(STATUS "Installing ${CURR_COMP}")
            execute_process(COMMAND ${CMAKE_COMMAND}
                            --install ${BIN_DIR}
                            --config ${CONFIG}
                            --component ${CURR_COMP}
                            --prefix ${INSTALL_DIR}
                            WORKING_DIRECTORY ${BIN_DIR}
                            COMMAND_ECHO STDOUT
            )
        endforeach()
    else()
        # Install everything
        execute_process(COMMAND ${CMAKE_COMMAND}
                        --install ${BIN_DIR}
                        --config ${CONFIG}
                        --prefix ${INSTALL_DIR}
                        WORKING_DIRECTORY ${BIN_DIR}
                        COMMAND_ECHO STDOUT
        )
    endif()
endfunction()

# Copy content DLLs
function(CopyContent TO_TARGET DLL_DEBUG DLL_RELEASE)
    # Compute if same directory for debug/release
    string(COMPARE EQUAL ${DLL_DEBUG} ${DLL_RELEASE} IN_SAME_DIR)
    # Gather dll files
    if(${IN_SAME_DIR})
        # Same directory -> Find by using dlls with "d" suffix
        file(GLOB DLL_LIST RELATIVE ${DLL_DEBUG} CONFIGURE_DEPENDS "${DLL_DEBUG}/*.dll")
        set(DLL_LIST_DEBUG ${DLL_LIST})
        set(DLL_LIST_RELEASE ${DLL_LIST})
        list(FILTER DLL_LIST_DEBUG INCLUDE REGEX ".*d\.dll")
        list(FILTER DLL_LIST_RELEASE EXCLUDE REGEX ".*d\.dll")
        # Override list if no debug dlls found
        list(LENGTH DLL_LIST_DEBUG DEBUG_COUNT)
        if(${DEBUG_COUNT} EQUAL 0)
            set(DLL_LIST_DEBUG ${DLL_LIST_RELEASE})
        endif()
    else()
        # Otherwise find all dlls in the directories
        file(GLOB DLL_LIST_DEBUG RELATIVE ${DLL_DEBUG} CONFIGURE_DEPENDS "${DLL_DEBUG}/*.dll")
        file(GLOB DLL_LIST_RELEASE RELATIVE ${DLL_RELEASE} CONFIGURE_DEPENDS "${DLL_RELEASE}/*.dll")
    endif()
    # Some info output
    message(STATUS "DLLs (debug) copied for ${TO_TARGET}: ${DLL_LIST_DEBUG}")
    message(STATUS "DLLs (release) copied for ${TO_TARGET}: ${DLL_LIST_RELEASE}")
    # Add post-build debug version copy commands to target
    foreach(DLL ${DLL_LIST_DEBUG})
        add_custom_command(TARGET ${TO_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,copy,true> "${DLL_DEBUG}/${DLL}" "${CMAKE_BINARY_DIR}/$<CONFIG>/${DLL}"
            VERBATIM
        )
    endforeach()
    # Same for release versions
    foreach(DLL ${DLL_LIST_RELEASE})
        add_custom_command(TARGET ${TO_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Release>,copy,true> "${DLL_RELEASE}/${DLL}" "${CMAKE_BINARY_DIR}/$<CONFIG>/${DLL}"
            VERBATIM
        )
    endforeach()
endfunction()
