function(AddPhysX TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME physx)
    include(ContentHelpers)
    include(FetchContent)

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
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/phcerdan/PhysX.git
                            GIT_TAG 4bdc673eba67ec7510a5c366b0c11b9f685d0ff3
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
            # Linux compiler needs to be Clang 9
            if(WIN32)
                set(EXTRA_FLAGS NV_USE_STATIC_WINCRT=OFF NV_USE_DEBUG_WINCRT=OFF)
                set(EXTRA_FLAGS_DEBUG NV_USE_STATIC_WINCRT=OFF NV_USE_DEBUG_WINCRT=ON)
            else()
                set(EXTRA_FLAGS CMAKE_C_COMPILER=clang-9 CMAKE_CXX_COMPILER=clang++-9 PX_GENERATE_SOURCE_DISTRO=ON NV_USE_GAMEWORKS_OUTPUT_DIRS=OFF)
            endif()
            # Configure physx release
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR}/physx ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        ${EXTRA_FLAGS}
            )
            # Build physx release
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH})

            # Debug only on Windows
            if(WIN32)
                # Configure physx debug
                CreateContent(${${CONTENT_NAME}_SOURCE_DIR}/physx ${${CONTENT_NAME}_BINARY_DIR}
                            CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                            ${EXTRA_FLAGS_DEBUG}
                )
                # Build physx debug
                BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
                InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH})
            endif()
        endif()
        # Load package with components
        CheckPhysX(CHECK_FOUND ${COMPONENTS})
    endif()

    # Find possible GPU dll dirs
    SubDirList(DEBUG_DIRS ${FETCHCONTENT_BASE_DIR}/${CONTENT_NAME}-src/physx/bin debug)
    SubDirList(RELEASE_DIRS ${FETCHCONTENT_BASE_DIR}/${CONTENT_NAME}-src/physx/bin release)

    # PhysX GPU libraries
    if(WIN32)
        # Find actual GPU dlls
        find_file(GPU_DEBUG_DLL
                PhysXGpu_64.dll
                PATHS
                ${DEBUG_DIRS}
                NO_DEFAULT_PATH
        )
        find_file(GPU_RELEASE_DLL
                PhysXGpu_64.dll
                HINTS
                ${RELEASE_DIRS}
                NO_DEFAULT_PATH
        )
        # Get directory of GPU dlls & add copy command
        get_filename_component(GPU_DEBUG_DLL ${GPU_DEBUG_DLL} DIRECTORY)
        get_filename_component(GPU_RELEASE_DLL ${GPU_RELEASE_DLL} DIRECTORY)
        CopyContent(${TO_TARGET} ${GPU_DEBUG_DLL} ${GPU_RELEASE_DLL})
    else()
        # Find actual GPU dll
        find_file(GPU_RELEASE_DLL
                libPhysXGpu_64.so
                HINTS
                ${RELEASE_DIRS}
                NO_DEFAULT_PATH
        )
        # Add copy command
        add_custom_command(TARGET ${TO_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GPU_RELEASE_DLL} $<TARGET_FILE_DIR:${TO_TARGET}>
            VERBATIM
        )
    endif()

    # Copy other required dlls
    if(WIN32)
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/PhysX/bin/debug ${INSTALL_PATH}/PhysX/bin/release)
    else()
        # Add copy dynamic libs to build directory
        file(GLOB DLL_LIST CONFIGURE_DEPENDS "${INSTALL_PATH}/PhysX/bin/release/*.so")
        add_custom_command(TARGET ${TO_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_LIST} $<TARGET_FILE_DIR:${TO_TARGET}>
            VERBATIM
        )
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

# Finds all subdirectories and adds suffix to each
function(SubDirList RESULT DIR SUFF)
    file(GLOB CHILDREN RELATIVE ${DIR} ${DIR}/*)
    foreach(CHILD ${CHILDREN})
        if(IS_DIRECTORY ${DIR}/${CHILD})
            set(DIRS "${DIRS};${DIR}/${CHILD}/${SUFF}")
        endif()
    endforeach()
    set(${RESULT} ${DIRS} PARENT_SCOPE)
endfunction()
