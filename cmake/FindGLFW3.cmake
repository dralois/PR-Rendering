# - Try to find GLFW
# Once done, this will define
#
# GLFW_FOUND        - system has GLFW
# GLFW_INCLUDE_DIRS - the GLFW include directories
# GLFW_LIBRARIES    - link these to use GLFW3

IF(GLFW_INCLUDE_DIR)
    SET(GLFW_FIND_QUIETLY TRUE)
ENDIF(GLFW_INCLUDE_DIR)

SET(GLFW_LIB_NAMES libglfw3.a glfw3 glfw GLFW3.lib)

IF(WIN32)
    FIND_PATH(GLFW_INCLUDE_DIR
            NAMES
            glfw3.h
            HINTS
            $ENV{GLFW_ROOT}/include
            $ENV{PROGRAMFILES}/GLFW/include
            ${PROJECT_SOURCE_DIR}/dependencies/include/GLFW
    )
    FIND_LIBRARY(GLFW_LIBRARY
                NAMES
                ${GLFW_LIB_NAMES}
                HINTS
                $ENV{GLFW_ROOT}/lib/lib-vc2017
                $ENV{GLFW_ROOT}/lib/lib-vc2019
                $ENV{PROGRAMFILES}/GLFW/lib/lib-vc2017
                $ENV{PROGRAMFILES}/GLFW/lib/lib-vc2019
                ${PROJECT_SOURCE_DIR}/dependencies/lib/lib-vc2017
                ${PROJECT_SOURCE_DIR}/dependencies/lib/lib-vc2019
    )
ELSE(WIN32)
    FIND_PATH(GLFW_INCLUDE_DIR
            NAMES
            GLFW/glfw3.h
            HINTS
            /sw/include
            /usr/include
            /usr/include/GLFW
            /opt/local/include
            /usr/local/include
            /usr/local/include/GLFW
            $ENV{GLFW_ROOT}/include
    )
    FIND_LIBRARY(GLFW_LIBRARY
                NAMES
                ${GLFW_LIB_NAMES}
                HINTS
                /sw/lib
                /usr/lib
                /usr/lib64
                /opt/local/lib
                /usr/local/lib
                /usr/local/lib64
                $ENV{GLFW_ROOT}/lib/lib-mingw-w64
    )
ENDIF(WIN32)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLFW DEFAULT_MSG GLFW_INCLUDE_DIR GLFW_LIBRARY)
MARK_AS_ADVANCED(GLFW_INCLUDE_DIR GLFW_LIBRARY)

IF(GLFW_INCLUDE_DIR AND GLFW_LIBRARY)
    SET(GLFW_FOUND TRUE)
    SET(GLFW_LIBRARIES ${GLFW_LIBRARY})
    SET(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
ENDIF(GLFW_INCLUDE_DIR AND GLFW_LIBRARY)
