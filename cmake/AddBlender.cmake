function(AddBlender TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME blender)
    include(ContentHelpers)

    # Check if package available
    CheckBlender(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        # Download source code
        if(WIN32)
            FetchContent_Declare(${CONTENT_NAME}
                                URL https://github.com/dralois/Blender-Python-Module-Docker/releases/download/v2.0/bpy-2.82a-windows.zip
                                URL_HASH MD5=4dbfcb47e1d93fe81138477064392bd1
            )
        else()
            FetchContent_Declare(${CONTENT_NAME}
                                URL https://github.com/dralois/Blender-Python-Module-Docker/releases/download/v2.0/bpy-2.82a-linux.zip
                                URL_HASH MD5=99ab0e484dfa7d360f62a57f42c4d713
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
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/bin ${INSTALL_PATH}/bin)
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
