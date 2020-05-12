function(AddAppleseed TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME appleseed)

    # Check if package available
    CheckAppleseed(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        include(ContentHelpers)
        # Download source code
        if(WIN32)
            FetchContent_Declare(${CONTENT_NAME}
                                URL https://github.com/appleseedhq/blenderseed/releases/download/2.0.0-beta/blenderseed-2.0.0-beta-0-g6b8f474-blender-2.80-win64.zip
                                URL_HASH MD5=1af7351efe5907a3b51dec139c4de5b6
            )
        else()
            # TODO
        endif()
        # TODO
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            # Configure appleseed
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
            )
            # Build appleseed
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH})
        endif()
        # Load package
        CheckAppleseed(CHECK_FOUND)
    endif()

    # TODO
    # Copy required dlls
    if(WIN32)
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/bin ${INSTALL_PATH}/bin)
    endif()

    # TODO
    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE appleseed::appleseed)
endfunction()

# TODO
function(CheckAppleseed CHECK_FOUND)
    # Try to load package
    find_package(appleseed
                PATHS
                ${INSTALL_PATH}
                NO_DEFAULT_PATH
    )
    # Return result
    set(CHECK_FOUND ${appleseed_FOUND} PARENT_SCOPE)
endfunction()
