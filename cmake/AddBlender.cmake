function(AddBlender TO_TARGET MODULE_DIR)
    # For reusability
    set(CONTENT_NAME blender)

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

    # Make available
    FetchContent_GetProperties(${CONTENT_NAME})
    if(NOT ${CONTENT_NAME}_POPULATED)
        FetchContent_Populate(${CONTENT_NAME})
    endif()

    # Add copy command
    add_custom_command(TARGET ${TO_TARGET} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${${CONTENT_NAME}_SOURCE_DIR} $<TARGET_FILE_DIR:${TO_TARGET}>
        VERBATIM
    )

    # Copy BlenderModule
    add_custom_command(TARGET ${TO_TARGET} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${MODULE_DIR} $<TARGET_FILE_DIR:${TO_TARGET}>/BlenderModule
        VERBATIM
    )
endfunction()
