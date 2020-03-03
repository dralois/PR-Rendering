# - Find OPENCV2
# Find the OPENCV2 includes and library
#
#  OPENCV2_INCLUDE_DIRS - Where to find OPENCV2 includes
#  OPENCV2_LIBRARIES    - List of libraries when using OPENCV2
#  OPENCV2_FOUND        - True if OPENCV2 was found

IF(OPENCV2_INCLUDE_DIR)
    SET(OPENCV2_FIND_QUIETLY TRUE)
ENDIF(OPENCV2_INCLUDE_DIR)

MACRO(FIND_OPENCV2_COMPONENT component version)

STRING(TOUPPER ${component} _uppercomponent)
SET(OPENCV_NAMES opencv_${component}${version} opencv_${component}420)
SET(OPENCV_DBG_NAMES opencv_${component}${version}d opencv_${component}420d)

INCLUDE(FindPackageHandleStandardArgs)

IF(WIN32)
    FIND_PATH(OPENCV2_INCLUDE_DIR
            NAMES
            "cv.h"
            "opencv.hpp"
            HINTS
            $ENV{PROGRAMFILES}/opencv2/include
            ${PROJECT_SOURCE_DIR}/dependencies/include/opencv2
    )
    FIND_LIBRARY(OPENCV2_${_uppercomponent}_LIBRARY
                NAMES
                ${OPENCV_NAMES}
                HINTS
                $ENV{PROGRAMFILES}/opencv2/lib/release
                ${PROJECT_SOURCE_DIR}/dependencies/lib/release
    )
    FIND_LIBRARY(OPENCV2_${_uppercomponent}_LIBRARY_DEBUG
                NAMES
                ${OPENCV_DBG_NAMES}
                HINTS
                $ENV{PROGRAMFILES}/opencv2/lib/debug
                ${PROJECT_SOURCE_DIR}/dependencies/lib/debug
    )

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENCV2_${_uppercomponent} DEFAULT_MSG OPENCV2_${_uppercomponent}_LIBRARY OPENCV2_${_uppercomponent}_LIBRARY_DEBUG OPENCV2_INCLUDE_DIR)
    MARK_AS_ADVANCED(OPENCV2_${_uppercomponent}_LIBRARY OPENCV2_${_uppercomponent}_LIBRARY_DEBUG OPENCV2_INCLUDE_DIR)

    IF(OPENCV2_${_uppercomponent}_LIBRARY_DEBUG AND OPENCV2_${_uppercomponent}_LIBRARY)
        SET(OPENCV2_LIBRARIES ${OPENCV2_LIBRARIES} optimized ${OPENCV2_${_uppercomponent}_LIBRARY} debug ${OPENCV2_${_uppercomponent}_LIBRARY_DEBUG})
    ENDIF(OPENCV2_${_uppercomponent}_LIBRARY_DEBUG AND OPENCV2_${_uppercomponent}_LIBRARY)
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
    FIND_LIBRARY(OPENCV2_${_uppercomponent}_LIBRARY
                NAMES
                ${OPENCV_NAMES}
                HINTS
                /sw/lib
                /usr/lib
                /usr/lib64
                /opt/local/lib
                /usr/local/lib
                /usr/local/lib64
    )

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENCV2_${_uppercomponent} DEFAULT_MSG OPENCV2_${_uppercomponent}_LIBRARY OPENCV2_INCLUDE_DIR)
    MARK_AS_ADVANCED(OPENCV2_${_uppercomponent}_LIBRARY OPENCV2_INCLUDE_DIR)

    SET(OPENCV2_LIBRARIES ${OPENCV2_LIBRARIES} ${OPENCV2_${_uppercomponent}_LIBRARY})
ENDIF(WIN32)

ENDMACRO(FIND_OPENCV2_COMPONENT)

FIND_OPENCV2_COMPONENT(core 420)
FIND_OPENCV2_COMPONENT(objdetect 420)
FIND_OPENCV2_COMPONENT(highgui 420)
FIND_OPENCV2_COMPONENT(imgproc 420)

IF(OPENCV2_CORE_FOUND AND OPENCV2_OBJDETECT_FOUND AND OPENCV2_HIGHGUI_FOUND AND OPENCV2_IMGPROC_FOUND)
    SET(OPENCV2_FOUND TRUE)
    SET(OPENCV2_INCLUDE_DIRS ${OPENCV2_INCLUDE_DIR})
ENDIF()
