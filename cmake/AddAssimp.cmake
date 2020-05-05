function(AddAssimp targetProject modulePath installPath)
    # For reusability
    set(CONTENT_NAME assimp)

    # Check if package available
    CheckAssimp(found)

    # Load and build if not so
    if(NOT ${found})
        # Enable dependency download module
        include(FetchContent)
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/assimp/assimp.git
                            GIT_TAG v5.0.0
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            include(${modulePath}/ContentHelpers.cmake)
            # Configure assimp
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        ASSIMP_BUILD_TESTS=OFF
                        ASSIMP_BUILD_ASSIMP_TOOLS=OFF
                        ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF
                        ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=OFF
                        ASSIMP_BUILD_OBJ_EXPORTER=ON
                        ASSIMP_BUILD_OBJ_IMPORTER=ON
            )
            # Build assimp
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${installPath})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${installPath})
        endif()
        # Load package
        CheckAssimp(found)
    endif()

    # Link and include components
    target_link_libraries(${targetProject} PRIVATE assimp::assimp)
endfunction()

function(CheckAssimp found)
    # Try to load package
    find_package(assimp
                PATHS
                ${installPath}
                NO_DEFAULT_PATH
    )
    # Return result
    set(found ${assimp_FOUND} PARENT_SCOPE)
endfunction()
