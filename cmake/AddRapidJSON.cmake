function(AddRapidJSON TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME rapidjson)
    include(ContentHelpers)

    # Check if package available
    CheckRapidJSON(CHECK_FOUND)

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            # Configure content
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        RAPIDJSON_BUILD_DOC=OFF
                        RAPIDJSON_BUILD_EXAMPLES=OFF
                        RAPIDJSON_BUILD_TESTS=OFF
            )
            # Build content
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH} dev)
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH} dev)
        endif()
        # Load package
        CheckRapidJSON(CHECK_FOUND)
    endif()

    # Link and include library
    target_link_libraries(${TO_TARGET} PRIVATE rapidjson)
endfunction()

function(CheckRapidJSON CHECK_FOUND)
    # Try to load package
    find_package(RapidJSON
                PATHS
                ${INSTALL_PATH}
                NO_DEFAULT_PATH
    )
    # Return result
    set(CHECK_FOUND ${RapidJSON_FOUND} PARENT_SCOPE)
endfunction()
