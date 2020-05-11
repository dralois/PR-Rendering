function(AddBlender TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME blender)

    # Check if package available
    CheckBlender(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        include(ContentHelpers)
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://git.blender.org/blender.git
                            GIT_TAG v2.82a
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Blender: Download precompiled dependencies
        if(WIN32)
            FetchContent_Declare(${CONTENT_NAME}_deps
                                SVN_REPOSITORY https://svn.blender.org/svnroot/bf-blender/trunk/lib/win64_vc15
                                SVN_TRUST_CERT True
            )
        else()
            FetchContent_Declare(${CONTENT_NAME}_deps
                                SVN_REPOSITORY https://svn.blender.org/svnroot/bf-blender/trunk/lib/linux_centos7_x86_64/
                                SVN_TRUST_CERT True
            )
        endif()
        # TODO
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            # Configure blender
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
            )
            # Build blender
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH})
        endif()
        # Load package
        CheckBlender(CHECK_FOUND)
    endif()

    # TODO
    # Copy required dlls
    if(WIN32)
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/bin ${INSTALL_PATH}/bin true)
    endif()

    # TODO
    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE blender::blender)
endfunction()

# TODO
function(CheckBlender CHECK_FOUND)
    # Try to load package
    find_package(blender
                PATHS
                ${INSTALL_PATH}
                NO_DEFAULT_PATH
    )
    # Return result
    set(CHECK_FOUND ${assimp_FOUND} PARENT_SCOPE)
endfunction()
