# - Try to find Assimp
# Once done, this will define
#
# ASSIMP_FOUND       - system has Assimp
# ASSIMP_INCLUDE_DIRS - the Assimp include directories
# ASSIMP_LIBRARIES   - link these to use Assimp

IF(ASSIMP_INCLUDE_DIR)
  SET(ASSIMP_FIND_QUIETLY TRUE)
ENDIF(ASSIMP_INCLUDE_DIR)

FIND_PATH(ASSIMP_INCLUDE_DIR
    NAMES
    mesh.h
    HINTS
    /usr/include
    /usr/local/include
    /opt/local/include
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/dependencies/assimp/include
    ${PROJECT_SOURCE_DIR}/dependencies/assimp/include
)

FIND_LIBRARY(ASSIMP_LIBRARY
    NAMES
    assimp
    assimp-vc140-mt
    HINTS
    /usr/lib64
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/dependencies/assimp/lib
    ${PROJECT_SOURCE_DIR}/dependencies/assimp/lib
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(assimp DEFAULT_MSG ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)
MARK_AS_ADVANCED(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)

IF(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY)
    SET(ASSIMP_FOUND TRUE)
    SET(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY})
    SET(ASSIMP_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
ENDIF(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY)
