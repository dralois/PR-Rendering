function(AddAppleseed TO_TARGET)
    # For reusability
    set(CONTENT_NAME appleseed)
    set(AS_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/${CONTENT_NAME}-src)

    # Enable dependency download module
    include(FetchContent)
    include(ExternalProject)

    # Download source code
    if(WIN32)
        FetchContent_Declare(${CONTENT_NAME}
                            URL https://github.com/dralois/Blender-Python-Module-Docker/releases/download/v2.0/blenderseed-windows.zip
                            URL_HASH MD5=bcaa490e54ba027c11640cc0bbe634b4
                            SOURCE_DIR ${AS_SOURCE_DIR}/blenderseed
        )
    else()
        FetchContent_Declare(${CONTENT_NAME}
                            URL https://github.com/dralois/Blender-Python-Module-Docker/releases/download/v2.0/blenderseed-linux.zip
                            URL_HASH MD5=bec012fa46a71ad07d72d64ecd454ae8
                            SOURCE_DIR ${AS_SOURCE_DIR}/blenderseed
        )
    endif()

    # Make available
    FetchContent_GetProperties(${CONTENT_NAME})
    if(NOT ${CONTENT_NAME}_POPULATED)
        FetchContent_Populate(${CONTENT_NAME})
        # Build zip
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E make_directory ${AS_SOURCE_DIR}/out
            COMMAND ${CMAKE_COMMAND} -E tar cfv ${AS_SOURCE_DIR}/out/blenderseed.zip --format=zip -- blenderseed
            WORKING_DIRECTORY ${AS_SOURCE_DIR}
        )
    endif()

    # Add copy Blenderseed
    add_custom_command(TARGET ${TO_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${AS_SOURCE_DIR}/out/blenderseed.zip $<TARGET_FILE_DIR:${TO_TARGET}>
        VERBATIM
    )

    # Add copy Appleseed library on Linux
    if(UNIX)
        add_custom_command(TARGET ${TO_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${${CONTENT_NAME}_SOURCE_DIR}/appleseed/lib/libappleseed.so $<TARGET_FILE_DIR:${TO_TARGET}>
            VERBATIM
        )
    endif()
endfunction()
