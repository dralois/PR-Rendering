function(AddRapidJSON targetProject modulePath installPath)
    # For reusability
    set(CONTENT_NAME rapidjson)

    # Check if package available
    CheckRapidJSON(found)

    # Load and build if not so
    if(NOT ${found})
        # Enable dependency download module
        include(FetchContent)
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
                            GIT_TAG v1.1.0
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            include(${modulePath}/ContentHelpers.cmake)
            # Configure content
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${installPath}
                        RAPIDJSON_BUILD_DOC=OFF
                        RAPIDJSON_BUILD_EXAMPLES=OFF
                        RAPIDJSON_BUILD_TESTS=OFF
            )
            # Build content
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${installPath})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${installPath})
        endif()
        # Load package
        CheckRapidJSON(found)
    endif()

    # Link and include directory
    target_include_directories(${targetProject} PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
endfunction()

function(CheckRapidJSON found)
    # Try to load package
    find_package(RapidJSON
                PATHS
                ${installPath}
                NO_DEFAULT_PATH
    )
    # Return result
    set(found ${RapidJSON_FOUND} PARENT_SCOPE)
endfunction()
