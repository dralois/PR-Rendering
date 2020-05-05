function(AddEigen targetProject modulePath installPath)
    # For reusability
    set(CONTENT_NAME eigen)

    # Check if package available
    CheckEigen(found)

    # Load and build if not so
    if(NOT ${found})
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
            include(${modulePath}/ContentHelpers.cmake)
            # Configure eigen
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${installPath}
                        BUILD_TESTING=OFF
            )
            # Build eigen
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${installPath})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${installPath})
        endif()
        # Load package
        CheckEigen(found)
    endif()

    # Link and include components
    target_link_libraries(${targetProject} PRIVATE Eigen3::Eigen)
endfunction()

function(CheckEigen found)
    # Try to load package
    find_package(Eigen3
                PATHS
                ${installPath}
                NO_DEFAULT_PATH
    )
    # Return result
    set(found ${Eigen3_FOUND} PARENT_SCOPE)
endfunction()
