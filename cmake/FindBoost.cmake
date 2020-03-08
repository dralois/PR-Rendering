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
    FIND_LIBRARY(Boost_LIBRARY_DEBUG
                NAMES
                libboost_filesystem-vc142-mt-gd-x64-1_72
                HINTS
                $ENV{PROGRAMFILES}/Boost/lib
                ${PROJECT_SOURCE_DIR}/dependencies/lib
    )

    INCLUDE(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG Boost_LIBRARY Boost_LIBRARY_DEBUG Boost_INCLUDE_DIR)
    MARK_AS_ADVANCED(Boost_LIBRARY Boost_LIBRARY_DEBUG Boost_INCLUDE_DIR)

    IF(Boost_LIBRARY_DEBUG AND Boost_LIBRARY)
        SET(Boost_LIBRARIES optimized ${Boost_LIBRARY} debug ${Boost_LIBRARY_DEBUG})
    ENDIF(Boost_LIBRARY_DEBUG AND Boost_LIBRARY)
ELSE(WIN32)
    FIND_PATH(Boost_INCLUDE_DIR
            NAMES
            filesystem.hpp
            version.hpp
            HINTS
            /sw/include
            /usr/include
            /usr/local/include
            /opt/local/include
    )
    FIND_LIBRARY(Boost_LIBRARY
        NAMES
        lboost_filesystem
        HINTS
        /sw/lib
        /usr/lib
        /usr/lib64
        /opt/local/lib
        /usr/local/lib
        /usr/local/lib64
    )

    INCLUDE(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG Boost_LIBRARY Boost_INCLUDE_DIR)
    MARK_AS_ADVANCED(Boost_LIBRARY Boost_INCLUDE_DIR)

    SET(Boost_LIBRARIES ${Boost_LIBRARY})
ENDIF(WIN32)

IF(Boost_INCLUDE_DIR AND Boost_LIBRARY)
    SET(Boost_FOUND TRUE)
    SET(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
ENDIF(Boost_INCLUDE_DIR AND Boost_LIBRARY)
