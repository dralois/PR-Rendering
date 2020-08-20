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

    # Find possible GPU dll dirs
    SubDirList(DEBUG_DIRS ${FETCHCONTENT_BASE_DIR}/${CONTENT_NAME}-src/physx/bin debug)
    SubDirList(RELEASE_DIRS ${FETCHCONTENT_BASE_DIR}/${CONTENT_NAME}-src/physx/bin release)

    # Find actual GPU dlls
    if(WIN32)
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
    else()
        find_file(GPU_DEBUG_DLL
                libPhysXGpu_64.so
                PATHS
                ${DEBUG_DIRS}
                NO_DEFAULT_PATH
        )
        find_file(GPU_RELEASE_DLL
                libPhysXGpu_64.so
                HINTS
                ${RELEASE_DIRS}
                NO_DEFAULT_PATH
        )
    endif()

    # Get directory of GPU dlls & add copy command
    get_filename_component(GPU_DEBUG_DLL ${GPU_DEBUG_DLL} DIRECTORY)
    get_filename_component(GPU_RELEASE_DLL ${GPU_RELEASE_DLL} DIRECTORY)
    CopyContent(${TO_TARGET} ${GPU_DEBUG_DLL} ${GPU_RELEASE_DLL})
    # Copy other required dlls
    CopyContent(${TO_TARGET} ${INSTALL_PATH}/PhysX/bin/debug ${INSTALL_PATH}/PhysX/bin/release)

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
