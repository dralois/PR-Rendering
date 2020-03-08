# - Try to find GLEW
# Once done, this will define
#
# GLEW_FOUND        - system has GLEW
# GLEW_INCLUDE_DIRS - the GLEW include directories
# GLEW_LIBRARIES    - link these to use GLEW

IF(GLEW_INCLUDE_DIR)
    SET(GLEW_FIND_QUIETLY TRUE)
ENDIF(GLEW_INCLUDE_DIR)

IF (WIN32)
    FIND_PATH(GLEW_INCLUDE_DIR
            NAMES
            GL/glew.h
            HINTS
            $ENV{PROGRAMFILES}/GLEW/include
            ${PROJECT_SOURCE_DIR}/dependencies/include/GLEW
    )
    FIND_LIBRARY(GLEW_LIBRARY
                NAMES
                glew32
                PATHS
                $ENV{PROGRAMFILES}/GLEW/lib
                ${PROJECT_SOURCE_DIR}/dependencies/lib
    )
ELSE (WIN32)
    FIND_PATH(GLEW_INCLUDE_DIR
            NAMES
            GL/glew.h
            HINTS
            /sw/include
            /usr/include
            /usr/local/include
            /opt/local/include
    )
    FIND_LIBRARY(GLEW_LIBRARY
                NAMES
                GLEW
                glew
                PATHS
                /sw/lib
                /usr/lib
                /usr/lib64
                /opt/local/lib
                /usr/local/lib
                /usr/local/lib64
    )
ENDIF (WIN32)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW DEFAULT_MSG GLEW_INCLUDE_DIR GLEW_LIBRARY)
MARK_AS_ADVANCED(GLEW_INCLUDE_DIR GLEW_LIBRARY)

IF(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
    SET(GLEW_FOUND TRUE)
    SET(GLEW_LIBRARIES ${GLEW_LIBRARY})
    SET(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
ENDIF(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
