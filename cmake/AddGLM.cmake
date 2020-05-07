function(AddGLM TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME glm)

    # Check if package available
    CheckGLM(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        # Download source code (to install folder)
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/g-truc/glm.git
                            GIT_TAG 0.9.9.8
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            # Configure glm
            set(GLM_TEST_ENABLE OFF CACHE INTERNAL "")
            set(BUILD_STATIC_LIBS OFF CACHE INTERNAL "")
            # Install glm (-> copy files because glm does not properly implement exporting)
            file(INSTALL ${${CONTENT_NAME}_SOURCE_DIR}/glm DESTINATION ${INSTALL_PATH} MESSAGE_NEVER
                FILES_MATCHING PATTERN "*.inl" PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.cpp")
            file(INSTALL ${${CONTENT_NAME}_SOURCE_DIR}/glm DESTINATION ${INSTALL_PATH}
                FILES_MATCHING PATTERN "*.txt")
            file(INSTALL ${${CONTENT_NAME}_SOURCE_DIR}/cmake DESTINATION ${INSTALL_PATH})
            file(INSTALL ${${CONTENT_NAME}_SOURCE_DIR}/CMakeLists.txt DESTINATION ${INSTALL_PATH})
        endif()
        # Load package
        CheckGLM(CHECK_FOUND)
    endif()

    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE glm::glm)
endfunction()

function(CheckGLM CHECK_FOUND)
    # Try to load package
    find_package(glm
                PATHS
                ${INSTALL_PATH}/cmake/glm
                NO_DEFAULT_PATH
    )
    # Return result
    set(CHECK_FOUND ${glm_FOUND} PARENT_SCOPE)
endfunction()
