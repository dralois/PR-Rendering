# - Try to find RapidJSON
# Once done, this will define
#
# RAPIDJSON_FOUND        - system has RAPIDJSON
# RAPIDJSON_INCLUDE_DIRS - the RAPIDJSON include directories

IF(RAPIDJSON_INCLUDE_DIR)
    SET(RAPIDJSON_FIND_QUIETLY TRUE)
ENDIF(RAPIDJSON_INCLUDE_DIR)

IF(WIN32)
    FIND_PATH(RAPIDJSON_INCLUDE_DIR
            NAMES
            rapidjson.h
            reader.h
            writer.h
            HINTS
            $ENV{PROGRAMFILES}/rapidjson/include
            ${PROJECT_SOURCE_DIR}/dependencies/include/rapidjson
    )
ELSE(WIN32)
    FIND_PATH(RAPIDJSON_INCLUDE_DIR
            NAMES
            rapidjson.h
            reader.h
            writer.h
            HINTS
            /sw/include
            /usr/include
            /usr/local/include
            /opt/local/include
    )
ENDIF(WIN32)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(RAPIDJSON DEFAULT_MSG RAPIDJSON_INCLUDE_DIR)
MARK_AS_ADVANCED(RAPIDJSON_INCLUDE_DIR)

IF(RAPIDJSON_INCLUDE_DIR)
    SET(RAPIDJSON_FOUND TRUE)
    SET(RAPIDJSON_INCLUDE_DIRS ${RAPIDJSON_INCLUDE_DIR})
ENDIF(RAPIDJSON_INCLUDE_DIR)
