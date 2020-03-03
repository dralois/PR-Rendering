# - Try to find Boost
# Once done, this will define
#
# Boost_FOUND        - system has Boost
# Boost_INCLUDE_DIRS - the Boost include directories
# Boost_LIBRARIES    - link these to use Boost

IF(Boost_INCLUDE_DIR)
    SET(Boost_FIND_QUIETLY TRUE)
ENDIF(Boost_INCLUDE_DIR)

IF(WIN32)
    FIND_PATH(Boost_INCLUDE_DIR
            NAMES
            filesystem.hpp
            version.hpp
            HINTS
            $ENV{PROGRAMFILES}/Boost/Boost
            ${PROJECT_SOURCE_DIR}/dependencies/include/boost
    )
    FIND_LIBRARY(Boost_LIBRARY
                NAMES
                libboost_filesystem-vc142-mt-x64-1_72
                HINTS
                $ENV{PROGRAMFILES}/Boost/lib
                ${PROJECT_SOURCE_DIR}/dependencies/lib
    )
ENDIF(WIN32)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG Boost_INCLUDE_DIR Boost_LIBRARY)
MARK_AS_ADVANCED(Boost_INCLUDE_DIR Boost_LIBRARY)

IF(Boost_INCLUDE_DIR AND Boost_LIBRARY)
    SET(Boost_FOUND TRUE)
    SET(Boost_LIBRARIES ${Boost_LIBRARY})
    SET(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
ENDIF(Boost_INCLUDE_DIR AND Boost_LIBRARY)
