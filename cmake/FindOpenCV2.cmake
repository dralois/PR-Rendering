# - Find OPENCV2
# Find the OPENCV2 includes and library
#
#  OPENCV2_INCLUDE_DIRS - Where to find OPENCV2 includes
#  OPENCV2_LIBRARIES    - List of libraries when using OPENCV2
#  OPENCV2_FOUND        - True if OPENCV2 was found

IF(OPENCV2_INCLUDE_DIR)
    SET(OPENCV2_FIND_QUIETLY TRUE)
ENDIF(OPENCV2_INCLUDE_DIR)

IF(WIN32)
    FIND_PATH(OPENCV2_INCLUDE_DIR
            NAMES
            "cv.h"
            "opencv.hpp"
            HINTS
            $ENV{PROGRAMFILES}/opencv2/include
            ${PROJECT_SOURCE_DIR}/dependencies/include/opencv2
    )
    FIND_LIBRARY(OPENCV2_LIBRARY
                NAMES
                opencv_world420
                HINTS
                $ENV{PROGRAMFILES}/opencv2/lib/release
                ${PROJECT_SOURCE_DIR}/dependencies/lib
    )
    FIND_LIBRARY(OPENCV2_LIBRARY_DEBUG
                NAMES
                opencv_world420d
                HINTS
                $ENV{PROGRAMFILES}/opencv2/lib/debug
                ${PROJECT_SOURCE_DIR}/dependencies/lib
    )

    INCLUDE(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENCV2 DEFAULT_MSG OPENCV2_LIBRARY OPENCV2_LIBRARY_DEBUG OPENCV2_INCLUDE_DIR)
    MARK_AS_ADVANCED(OPENCV2_LIBRARY OPENCV2_LIBRARY_DEBUG OPENCV2_INCLUDE_DIR)

    IF(OPENCV2_LIBRARY_DEBUG AND OPENCV2_LIBRARY)
        SET(OPENCV2_LIBRARIES optimized ${OPENCV2_LIBRARY} debug ${OPENCV2_LIBRARY_DEBUG})
    ENDIF(OPENCV2_LIBRARY_DEBUG AND OPENCV2_LIBRARY)
ELSE(WIN32)
    FIND_PATH(OPENCV2_INCLUDE_DIR
            NAMES
            "cv.h"
            "opencv.hpp"
            HINTS
            /sw/include
            /usr/include
            /usr/local/include
            /opt/local/include
    )
    FIND_LIBRARY(OPENCV2_LIBRARY
                NAMES
                opencv_world420
                HINTS
                /sw/lib
                /usr/lib
                /usr/lib64
                /opt/local/lib
                /usr/local/lib
                /usr/local/lib64
    )

    INCLUDE(FindPackageHandleStandardArgs)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENCV2 DEFAULT_MSG OPENCV2_LIBRARY OPENCV2_INCLUDE_DIR)
    MARK_AS_ADVANCED(OPENCV2_LIBRARY OPENCV2_INCLUDE_DIR)

    SET(OPENCV2_LIBRARIES ${OPENCV2_LIBRARY})
ENDIF(WIN32)

IF(OPENCV2_INCLUDE_DIR AND OPENCV2_LIBRARY)
    SET(OPENCV2_FOUND TRUE)
    SET(OPENCV2_INCLUDE_DIRS ${OPENCV2_INCLUDE_DIR})
ENDIF(OPENCV2_INCLUDE_DIR AND OPENCV2_LIBRARY)
