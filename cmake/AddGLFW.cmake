function(AddGLFW TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME glfw)
    
    # Check if package available
    CheckGLFW(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        include(ContentHelpers)
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
            # Configure GLFW
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        BUILD_SHARED_LIBS=ON
                        GLFW_BUILD_DOCS=OFF
                        GLFW_BUILD_EXAMPLES=OFF
                        GLFW_BUILD_TESTS=OFF
            )
            # Build GLFW
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH}/release)
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH}/debug)
        endif()
        # Load package
        CheckGLFW(CHECK_FOUND)
    endif()

    # Copy required dlls
    if(WIN32)
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/debug/bin ${INSTALL_PATH}/release/bin)
    endif()

    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE glfw)
endfunction()

function(CheckGLFW CHECK_FOUND)
    # Determine if GLFW is installed
    if((EXISTS ${INSTALL_PATH}/release/lib/cmake/glfw3/glfw3Targets.cmake) AND
        (EXISTS ${INSTALL_PATH}/debug/lib/cmake/glfw3/glfw3Targets-debug.cmake))
        # Load packages (-> Hack, GLFW does not properly implement configs)
        include(${INSTALL_PATH}/release/lib/cmake/glfw3/glfw3Targets.cmake)
        set(_IMPORT_PREFIX ${INSTALL_PATH}/debug)
        include(${INSTALL_PATH}/debug/lib/cmake/glfw3/glfw3Targets-debug.cmake)
        set(_IMPORT_PREFIX)
        # Return result
        set(CHECK_FOUND 1 PARENT_SCOPE)
    else()
        # Return result
        set(CHECK_FOUND 0 PARENT_SCOPE)
    endif()
endfunction()
