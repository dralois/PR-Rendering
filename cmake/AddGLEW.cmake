function(AddGLEW targetProject modulePath installPath)
    # For reusability
    set(CONTENT_NAME glew)

    # Check if package available
    CheckGLEW(found)

    # Load and build if not so
    if(NOT ${found})
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
            include(${modulePath}/ContentHelpers.cmake)
            # Update default CMakeLists.txt (-> MSVC compile bug)
            file(STRINGS ${${CONTENT_NAME}_SOURCE_DIR}/build/cmake/CMakeLists.txt LINES)
            file(WRITE ${${CONTENT_NAME}_SOURCE_DIR}/build/cmake/CMakeLists.txt "")
            foreach(LINE IN LISTS LINES)
                string(REPLACE "target_link_libraries (glew LINK_PRIVATE -nodefaultlib -noentry)" "" STRIPPED "${LINE}")
                string(REPLACE "target_link_libraries (glew LINK_PRIVATE -BASE:0x62AA0000)" "" STRIPPED "${LINE}")
                file(APPEND ${${CONTENT_NAME}_SOURCE_DIR}/build/cmake/CMakeLists.txt "${STRIPPED}\n")
            endforeach()
            # Configure GLEW
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR}/build/cmake ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${installPath}
                        BUILD_UTILS=OFF
            )
            # Build GLEW
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${installPath})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${installPath})
        endif()
        # Load package
        CheckGLEW(found)
    endif()

    # Link and include components
    target_link_libraries(${targetProject} PRIVATE GLEW::glew)
endfunction()

function(CheckGLEW found)
    # Try to load package
    find_package(GLEW
                PATHS
                ${installPath}
                NO_DEFAULT_PATH
    )
    # Return result
    set(found ${GLEW_FOUND} PARENT_SCOPE)
endfunction()
