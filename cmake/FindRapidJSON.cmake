# - Try to find RapidJSON
# Once done, this will define
#
# RAPIDJSON_FOUND        - system has RAPIDJSON
# RAPIDJSON_INCLUDE_DIRS - the RAPIDJSON include directories

IF(RAPIDJSON_INCLUDE_DIR)
    SET(RAPIDJSON_FIND_QUIETLY TRUE)
ENDIF(RAPIDJSON_INCLUDE_DIR)

FIND_PATH( RAPIDJSON_INCLUDE_DIR
    NAMES
    rapidjson.h
    reader.h
    writer.h
    HINTS
    /usr/include
    /usr/local/include
    /opt/local/include
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/dependencies/rapidjson/include
    ${PROJECT_SOURCE_DIR}/dependencies/rapidjson/include
    PATH_SUFFIXES
    include
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(RAPIDJSON DEFAULT_MSG RAPIDJSON_INCLUDE_DIR)
MARK_AS_ADVANCED(RAPIDJSON_INCLUDE_DIR)

IF(RAPIDJSON_INCLUDE_DIR)
    SET(RAPIDJSON_FOUND TRUE)
    SET(RAPIDJSON_INCLUDE_DIRS ${RAPIDJSON_INCLUDE_DIR})
ENDIF(RAPIDJSON_INCLUDE_DIR)
