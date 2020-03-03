# - Try to find glm
# Once done, this will define
#
# glm_FOUND        - system has glm
# glm_INCLUDE_DIRS - the glm include directories
# glm_LIBRARIES    - link these to use glm

IF(glm_INCLUDE_DIR)
    SET(glm_FIND_QUIETLY TRUE)
ENDIF(glm_INCLUDE_DIR)

IF (WIN32)
    FIND_PATH(glm_INCLUDE_DIR
            NAMES
            glm.hpp
            HINTS
            $ENV{PROGRAMFILES}/glm/include
            ${PROJECT_SOURCE_DIR}/dependencies/include/glm
    )
    FIND_LIBRARY(glm_LIBRARY
                NAMES
                glm_static
                PATHS
                $ENV{PROGRAMFILES}/glm/lib
                ${PROJECT_SOURCE_DIR}/dependencies/lib
    )
ELSE (WIN32)
    FIND_PATH(glm_INCLUDE_DIR
            NAMES
            glm.hpp
            HINTS
            /sw/include
            /usr/include
            /usr/local/include
            /opt/local/include
    )
    FIND_LIBRARY(glm_LIBRARY
                NAMES
                glm_static
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

FIND_PACKAGE_HANDLE_STANDARD_ARGS(glm DEFAULT_MSG glm_INCLUDE_DIR glm_LIBRARY)
MARK_AS_ADVANCED(glm_INCLUDE_DIR glm_LIBRARY)

IF(glm_INCLUDE_DIR AND glm_LIBRARY)
    SET(glm_FOUND TRUE)
    SET(glm_LIBRARIES ${glm_LIBRARY})
    SET(glm_INCLUDE_DIRS ${glm_INCLUDE_DIR})
ENDIF(glm_INCLUDE_DIR AND glm_LIBRARY)
