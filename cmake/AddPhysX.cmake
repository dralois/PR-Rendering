function(AddPhysX TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME physx)

    # Parse components
    if(${ARGC} GREATER 2)
        foreach(CURR_COMP ${ARGN})
            set(COMPONENTS ${COMPONENTS} ${CURR_COMP})
        endforeach()
    endif()

    # Check if package with components available
    CheckPhysX(CHECK_FOUND ${COMPONENTS})

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        include(ContentHelpers)
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
            # Update WINCRT to work when not built static (-> physx cmake bug)
            file(STRINGS ${${CONTENT_NAME}_SOURCE_DIR}/physx/source/CMakeLists.txt FILE_LINES)
            file(WRITE ${${CONTENT_NAME}_SOURCE_DIR}/physx/source/CMakeLists.txt "")
            foreach(LINE IN LISTS FILE_LINES)
                string(COMPARE EQUAL "string(REGEX REPLACE \"md\" \"mt\" PLATFORM_BIN_NAME \"\${PLATFORM_BIN_NAME}\")" "${LINE}" IS_REPLACE)
                if(NOT ${IS_REPLACE})
                    string(REPLACE "GetPlatformBinName\(PLATFORM_BIN_NAME \${LIBPATH_SUFFIX})"
                            "GetPlatformBinName(PLATFORM_BIN_NAME \${LIBPATH_SUFFIX})\nstring(REGEX REPLACE \"md\" \"mt\" PLATFORM_BIN_NAME \"\${PLATFORM_BIN_NAME}\")"
                            STRIPPED "${LINE}"
                    )
                    file(APPEND ${${CONTENT_NAME}_SOURCE_DIR}/physx/source/CMakeLists.txt "${STRIPPED}\n")
                endif()
            endforeach()
            # Configure physx release
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR}/physx ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        NV_USE_STATIC_WINCRT=OFF
                        NV_USE_DEBUG_WINCRT=OFF
            )
            # Build physx release
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH})
            # Configure physx debug
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR}/physx ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        NV_USE_STATIC_WINCRT=OFF
                        NV_USE_DEBUG_WINCRT=ON
            )
            # Build physx debug
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH})
        endif()
        # Load package with components
        CheckPhysX(CHECK_FOUND ${COMPONENTS})
    endif()

    # Copy required dlls
    if(WIN32)
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/PhysX/bin/debug ${INSTALL_PATH}/PhysX/bin/release)
    endif()

    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE ${COMPONENTS})
endfunction()

function(CheckPhysX CHECK_FOUND)
    # Search for components or in general
    if(${ARGC} GREATER 1)
        find_package(PhysX
                    COMPONENTS
                    ${ARGN}
                    PATHS
                    ${INSTALL_PATH}/PhysX/bin/cmake/physx
                    NO_DEFAULT_PATH
        )
    else()
        find_package(PhysX
                    PATHS
                    ${INSTALL_PATH}/PhysX/bin/cmake/physx
                    NO_DEFAULT_PATH
        )
    endif()
    # Return result
    set(CHECK_FOUND ${PhysX_FOUND} PARENT_SCOPE)
endfunction()
