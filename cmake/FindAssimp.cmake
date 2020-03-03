# - Try to find Assimp
# Once done, this will define
#
# Assimp_FOUND        - system has Assimp
# Assimp_INCLUDE_DIRS - the Assimp include directories
# Assimp_LIBRARIES    - link these to use Assimp

IF(Assimp_INCLUDE_DIR)
    SET(Assimp_FIND_QUIETLY TRUE)
ENDIF(Assimp_INCLUDE_DIR)

IF(WIN32)
    FIND_PATH(Assimp_INCLUDE_DIR
            NAMES
            mesh.h
            HINTS
            $ENV{PROGRAMFILES}/Assimp/include
            ${PROJECT_SOURCE_DIR}/dependencies/include/Assimp
    )
    FIND_LIBRARY(Assimp_LIBRARY
                NAMES
                Assimp
                Assimp-vc120-mt
                Assimp-vc140-mt
                HINTS
                $ENV{PROGRAMFILES}/Assimp/lib
                ${PROJECT_SOURCE_DIR}/dependencies/lib
    )
ELSE(WIN32)
    FIND_PATH(Assimp_INCLUDE_DIR
            NAMES
            mesh.h
            HINTS
            /sw/include
            /usr/include
            /usr/local/include
            /opt/local/include
    )
    FIND_LIBRARY(Assimp_LIBRARY
                NAMES
                Assimp
                HINTS
                /sw/lib
                /usr/lib
                /usr/lib64
                /opt/local/lib
                /usr/local/lib
                /usr/local/lib64
    )
ENDIF(WIN32)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Assimp DEFAULT_MSG Assimp_INCLUDE_DIR Assimp_LIBRARY)
MARK_AS_ADVANCED(Assimp_INCLUDE_DIR Assimp_LIBRARY)

IF(Assimp_INCLUDE_DIR AND Assimp_LIBRARY)
    SET(Assimp_FOUND TRUE)
    SET(Assimp_LIBRARIES ${Assimp_LIBRARY})
    SET(Assimp_INCLUDE_DIRS ${Assimp_INCLUDE_DIR})
ENDIF(Assimp_INCLUDE_DIR AND Assimp_LIBRARY)
