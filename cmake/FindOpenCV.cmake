# - Find OPENCV2
# Find the OPENCV2 includes and library
#
#  OPENCV2_INCLUDE_DIRS - Where to find OPENCV2 includes
#  OPENCV2_LIBRARIES    - List of libraries when using OPENCV2
#  OPENCV2_FOUND        - True if OPENCV2 was found

IF(OPENCV2_INCLUDE_DIR)
    SET(OPENCV2_FIND_QUIETLY TRUE)
ENDIF(OPENCV2_INCLUDE_DIR)

FIND_PATH(OPENCV2_INCLUDE_DIR
    NAMES
    "cv.h"
    "opencv.hpp"
    HINTS
    /usr/include
    /usr/local/include
    /opt/local/include
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/dependencies/opencv2/include
    ${PROJECT_SOURCE_DIR}/dependencies/opencv2/include
)

INCLUDE(FindPackageHandleStandardArgs)

MACRO(FIND_OPENCV2_COMPONENT component version)

STRING(TOUPPER ${component} _uppercomponent)
SET(OPENCV2_NAMES opencv_${component}${version} opencv_${component}420)
SET(OPENCV2_WOV_NAMES opencv_${component})
SET(OPENCV2_DBG_NAMES opencv_${component}${version}d opencv_${component}420d)

FIND_LIBRARY(OPENCV2_${_uppercomponent}_LIBRARY
    NAMES
    ${OPENCV2_NAMES}
    HINTS
    /usr/lib64
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/release
    ${CMAKE_SOURCE_DIR}/dependencies/opencv2/lib/release
    ${PROJECT_SOURCE_DIR}/dependencies/opencv2/lib/release
    PATH_SUFFIXES
    lib
    lib64
    include
    release
)

IF(MSVC)
    # VisualStudio needs a debug version
    FIND_LIBRARY(OPENCV2_${_uppercomponent}_LIBRARY_DEBUG
        NAMES
        ${OPENCV2_DBG_NAMES}
        HINTS
        /usr/lib64
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        ${CMAKE_SOURCE_DIR}/lib/debug
        ${CMAKE_SOURCE_DIR}/dependencies/opencv2/lib/debug
        ${PROJECT_SOURCE_DIR}/dependencies/opencv2/lib/debug
        PATH_SUFFIXES
        lib
        lib64
        include
        debug
    )

    IF(OPENCV2_${_uppercomponent}_LIBRARY_DEBUG AND OPENCV2_${_uppercomponent}_LIBRARY)
        SET(OPENCV2_LIBRARIES ${OPENCV2_LIBRARIES} optimized ${OPENCV2_${_uppercomponent}_LIBRARY} debug ${OPENCV2_${_uppercomponent}_LIBRARY_DEBUG})
    ENDIF(OPENCV2_${_uppercomponent}_LIBRARY_DEBUG AND OPENCV2_${_uppercomponent}_LIBRARY)

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENCV2_${_uppercomponent} DEFAULT_MSG OPENCV2_${_uppercomponent}_LIBRARY OPENCV2_${_uppercomponent}_LIBRARY_DEBUG OPENCV2_INCLUDE_DIR)

    MARK_AS_ADVANCED(OPENCV2_${_uppercomponent}_LIBRARY OPENCV2_${_uppercomponent}_LIBRARY_DEBUG OPENCV2_INCLUDE_DIR)

ELSE(MSVC)
    # rest of the world

    FIND_LIBRARY(OPENCV2_${_uppercomponent}_LIBRARY
        NAMES
        ${OPENCV2_WOV_NAMES}
        HINTS
        /usr/lib64
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        ${CMAKE_SOURCE_DIR}/lib
    )

    SET(OPENCV2_LIBRARIES ${OPENCV2_LIBRARIES} ${OPENCV2_${_uppercomponent}_LIBRARY})

    FIND_PACKAGE_HANDLE_STANDARD_ARGS(OPENCV2_${_uppercomponent} DEFAULT_MSG OPENCV2_${_uppercomponent}_LIBRARY OPENCV2_INCLUDE_DIR)

    MARK_AS_ADVANCED(OPENCV2_${_uppercomponent}_LIBRARY OPENCV2_INCLUDE_DIR)

ENDIF(MSVC)

ENDMACRO(FIND_OPENCV2_COMPONENT)

FIND_OPENCV2_COMPONENT(core 2412)
FIND_OPENCV2_COMPONENT(objdetect 2412)
FIND_OPENCV2_COMPONENT(highgui 2412)
FIND_OPENCV2_COMPONENT(imgproc 2412)

IF(OPENCV2_CORE_FOUND AND OPENCV2_OBJDETECT_FOUND AND OPENCV2_HIGHGUI_FOUND AND OPENCV2_IMGPROC_FOUND)
    SET(OPENCV2_FOUND TRUE)
ENDIF()

IF(OPENCV2_FOUND)
    SET(OPENCV2_INCLUDE_DIRS ${OPENCV2_INCLUDE_DIR})
ENDIF(OPENCV2_FOUND)