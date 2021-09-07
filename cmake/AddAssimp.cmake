function(AddAssimp TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME assimp)
    include(ContentHelpers)

    # Check if package available
    CheckAssimp(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
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
            # Configure assimp
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        BUILD_SHARED_LIBS=1
                        ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF
                        ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=OFF
                        ASSIMP_BUILD_OBJ_EXPORTER=ON
                        ASSIMP_BUILD_OBJ_IMPORTER=ON
                        ASSIMP_BUILD_PLY_IMPORTER=ON
                        ASSIMP_BUILD_GLTF_IMPORTER=ON
                        ASSIMP_BUILD_ASSIMP_TOOLS=OFF
                        ASSIMP_BUILD_TESTS=OFF
                        ASSIMP_INSTALL=OFF
            )
            # Build assimp
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH})

            # Debug only on Windows
            if(WIN32)
                BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
                InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH})
            endif()
        endif()
        # Load package
        CheckAssimp(CHECK_FOUND)
    endif()

    # Copy required dlls
    if(WIN32)
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/bin ${INSTALL_PATH}/bin)
    endif()

    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE assimp::assimp)
endfunction()

function(CheckAssimp CHECK_FOUND)
    # Try to load package
    find_package(assimp
                PATHS
                ${INSTALL_PATH}
                PATH_SUFFIXES
                lib
                NO_DEFAULT_PATH
    )
    # Return result
    set(CHECK_FOUND ${assimp_FOUND} PARENT_SCOPE)
endfunction()
