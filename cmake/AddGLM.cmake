function(AddGLM targetProject modulePath installPath)
    # For reusability
    set(CONTENT_NAME glm)

    # Check if package available
    CheckGLM(found)

    # Load and build if not so
    if(NOT ${found})
        # Enable dependency download module
        include(FetchContent)
        # Download source code (to install folder)
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/g-truc/glm.git
                            GIT_TAG 0.9.9.8
                            GIT_SHALLOW True
                            GIT_PROGRESS True
                            SOURCE_DIR ${installPath}
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            # Configure glm
            set(GLM_TEST_ENABLE OFF CACHE INTERNAL "")
            set(BUILD_STATIC_LIBS OFF CACHE INTERNAL "")
        endif()
        # Load package
        CheckGLM(found)
    endif()

    # Link and include components
    target_link_libraries(${targetProject} PRIVATE glm::glm)
endfunction()

function(CheckGLM found)
    # Try to load package
    find_package(glm
                PATHS
                ${installPath}/cmake/glm
                NO_DEFAULT_PATH
    )
    # Return result
    set(found ${glm_FOUND} PARENT_SCOPE)
endfunction()
