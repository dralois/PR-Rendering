# - Try to find EIGEN3
# Once done, this will define
#
# EIGEN3_FOUND       - system has EIGEN3
# EIGEN3_INCLUDE_DIRS - the EIGEN3 include directories

IF(EIGEN3_INCLUDE_DIR)
  SET(EIGEN3_FIND_QUIETLY TRUE)
ENDIF(EIGEN3_INCLUDE_DIR)

FIND_PATH(EIGEN3_INCLUDE_DIR
    NAMES
    Eigen
    Dense
    PATH_SUFFIXES
    include
    Eigen
    src
    HINTS
    /usr/include
    /usr/local/include
    /opt/local/include
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/dependencies/eigen/include
    ${PROJECT_SOURCE_DIR}/dependencies/eigen/include
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(EIGEN3 DEFAULT_MSG EIGEN3_INCLUDE_DIR)
MARK_AS_ADVANCED(EIGEN3_INCLUDE_DIR)

IF(EIGEN3_INCLUDE_DIR)
    SET(EIGEN3_FOUND TRUE)
    SET(EIGEN3_INCLUDE_DIRS ${EIGEN3_INCLUDE_DIR})
ENDIF(EIGEN3_INCLUDE_DIR)
