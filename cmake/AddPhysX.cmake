function(AddPhysX targetProject modulePath installPath)
    # For reusability
    set(CONTENT_NAME physx)

    # Check if package available
    CheckPhysX(found)

    # Load and build if not so
    if(NOT ${found})
        # Enable dependency download module
        include(FetchContent)
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/phcerdan/PhysX.git
                            GIT_TAG cmake_for_easier_integration
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            include(${modulePath}/ContentHelpers.cmake)
            # Configure physx
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR}/physx ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${installPath})
            # Build physx
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${installPath})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${installPath})
        endif()
        # Load package
        CheckPhysX(found)
    endif()

    # Parse components
    if(${ARGC} GREATER 3)
        foreach(comp ${ARGN})
            set(COMPONENTS ${COMPONENTS} ${comp})
        endforeach()
    endif()

    # Link and include components
    target_link_libraries(${targetProject} PRIVATE ${COMPONENTS})
endfunction()

function(CheckPhysX found)
    # Try to load package
    find_package(PhysX
                PATHS
                ${installPath}/PhysX/bin/cmake/physx
                NO_DEFAULT_PATH
    )
    # Return result
    set(found ${PhysX_FOUND} PARENT_SCOPE)
endfunction()
