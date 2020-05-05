function(AddGLFW targetProject modulePath installPath)
    # For reusability
    set(CONTENT_NAME glfw)
    
    # Check if package available
    CheckGLFW(found)

    # Load and build if not so
    if(NOT ${found})
        # Enable dependency download module
        include(FetchContent)
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/glfw/glfw.git
                            GIT_TAG 3.3.2
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            include(${modulePath}/ContentHelpers.cmake)
            # Configure GLFW
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${installPath}
                        BUILD_SHARED_LIBS=ON
                        GLFW_BUILD_DOCS=OFF
                        GLFW_BUILD_EXAMPLES=OFF
                        GLFW_BUILD_TESTS=OFF
            )
            # Build GLFW
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${installPath}/release)
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${installPath}/debug)
        endif()
        # Load package
        CheckGLFW(found)
    endif()

    # Link and include components
    target_link_libraries(${targetProject} PRIVATE glfw)
endfunction()

function(CheckGLFW found)
    # Determine if GLFW is installed
    if((EXISTS ${installPath}/release/lib/cmake/glfw3/glfw3Targets.cmake) AND
        (EXISTS ${installPath}/debug/lib/cmake/glfw3/glfw3Targets-debug.cmake))
        # Load packages (-> Hack, GLFW does not properly implement configs)
        include(${installPath}/release/lib/cmake/glfw3/glfw3Targets.cmake)
        set(_IMPORT_PREFIX ${installPath}/debug)
        include(${installPath}/debug/lib/cmake/glfw3/glfw3Targets-debug.cmake)
        set(_IMPORT_PREFIX)
        # Return result
        set(found 1 PARENT_SCOPE)
    else()
        # Return result
        set(found 0 PARENT_SCOPE)
    endif()
endfunction()
