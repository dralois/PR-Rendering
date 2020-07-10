function(AddGLEW TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME glew)
    include(ContentHelpers)

    # Check if package available
    CheckGLEW(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        # Download source code (complete release)
        FetchContent_Declare(${CONTENT_NAME}
                            URL https://github.com/nigels-com/glew/releases/download/glew-2.1.0/glew-2.1.0.zip
                            URL_HASH MD5=dff2939fd404d054c1036cc0409d19f1
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            # Update default CMakeLists.txt (-> MSVC compile bug)
            file(STRINGS ${${CONTENT_NAME}_SOURCE_DIR}/build/cmake/CMakeLists.txt FILE_LINES)
            file(WRITE ${${CONTENT_NAME}_SOURCE_DIR}//build/cmake/CMakeLists.txt "")
            foreach(LINE IN LISTS FILE_LINES)
                string(REGEX REPLACE "(.*-nodefaultlib.*)|(.*-noentry.*)|(.*-BASE:0x62AA0000.*)" "" STRIPPED "${LINE}")
                file(APPEND ${${CONTENT_NAME}_SOURCE_DIR}/build/cmake/CMakeLists.txt "${STRIPPED}\n")
            endforeach()
            # Configure GLEW
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR}/build/cmake ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        BUILD_UTILS=OFF
            )
            # Build GLEW
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH})
        endif()
        # Load package
        CheckGLEW(CHECK_FOUND)
    endif()

    # Copy required dlls
    if(WIN32)
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/bin ${INSTALL_PATH}/bin)
    endif()

    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE GLEW::glew)
endfunction()

function(CheckGLEW CHECK_FOUND)
    # Try to load package
    find_package(GLEW
                PATHS
                ${INSTALL_PATH}
                NO_DEFAULT_PATH
    )
    # Return result
    set(CHECK_FOUND ${GLEW_FOUND} PARENT_SCOPE)
endfunction()
