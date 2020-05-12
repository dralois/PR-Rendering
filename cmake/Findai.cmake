# Quiet exit
if(AI_INCLUDE_DIR)
  set(AI_FIND_QUIETLY TRUE)
endif()

# Search for includes / lib
if(WIN32)
    find_path(AI_INCLUDE_DIR
            NAMES
            ai.h
            HINTS
            ${PROJECT_EXTERNAL_DIR}/arnold/include
            ${CMAKE_SOURCE_DIR}/external/arnold/include
    )
    find_library(AI_LIBRARY
                NAMES
                ai
                HINTS
                ${PROJECT_EXTERNAL_DIR}/arnold/lib
                ${CMAKE_SOURCE_DIR}/external/arnold/lib
    )
else()
    find_path(AI_INCLUDE_DIR
            NAMES
            ai.h
            HINTS
            /sw/include
            /usr/include
            /usr/local/include
            /opt/local/include
            /usr/local/include/ai
    )
    find_library(AI_LIBRARY
                NAMES
                ai
                HINTS
                /sw/lib
                /usr/lib
                /usr/lib64
                /opt/local/lib
                /usr/local/lib
                /usr/local/lib64
    )
endif()

# Standard find_package stuff
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AI DEFAULT_MSG AI_INCLUDE_DIR AI_LIBRARY)
mark_as_advanced(FORCE AI_INCLUDE_DIR AI_LIBRARY)

# Setup target
if(AI_FOUND AND NOT TARGET AI::AI)
    # Make Arnold as imported target available
    add_library(AI::AI UNKNOWN IMPORTED)
    set_target_properties(AI::AI PROPERTIES
        IMPORTED_LOCATION "${AI_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${AI_INCLUDE_DIR}"
    )
    # Copy required dlls into build folder
    if(WIN32)
        include(ContentHelpers)
        # Add copy target
        add_custom_target(CopyDlls${PROJECT_NAME} COMMENT "Copies required dlls" VERBATIM)
        # Add copy commands
        CopyContent(CopyDlls${PROJECT_NAME} ${AI_INCLUDE_DIR}/../bin ${AI_INCLUDE_DIR}/../bin)
        # Make library depend on it
        add_dependencies(AI::AI CopyDlls${PROJECT_NAME})
        # Hide in IDE folder
        set_target_properties(CopyDlls${PROJECT_NAME} PROPERTIES FOLDER "DLL Copy")
    endif()
endif()
