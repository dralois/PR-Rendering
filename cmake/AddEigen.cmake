function(AddEigen TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME eigen)
    include(ContentHelpers)

    # Check if package available
    CheckEigen(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
                            GIT_TAG 3.3.7
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            # Configure eigen
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        BUILD_TESTING=OFF
            )
            # Build eigen
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH})
        endif()
        # Load package
        CheckEigen(CHECK_FOUND)
    endif()

    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE Eigen3::Eigen)
endfunction()

function(CheckEigen CHECK_FOUND)
    # Try to load package
    find_package(Eigen3
                PATHS
                ${INSTALL_PATH}
                NO_DEFAULT_PATH
    )
    # Return result
    set(CHECK_FOUND ${Eigen3_FOUND} PARENT_SCOPE)
endfunction()
