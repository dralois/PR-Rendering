# - Find Arnold SDK
# Find the Arnold SDK includes and library
#
#  AI_INCLUDE_DIRS - Where to find Arnold SDK includes
#  AI_LIBRARIES    - List of libraries when using Arnold SDK
#  AI_FOUND        - True if Arnold SDK was found

IF(AI_INCLUDE_DIR)
  SET(AI_FIND_QUIETLY TRUE)
ENDIF(AI_INCLUDE_DIR)

FIND_PATH(AI_INCLUDE_DIR
    NAMES
    ai.h
    HINTS
    /home/fabi/Downloads/Arnold-5.4/Arnold-5.4.0.2-linux/include
    /usr/include/
    /usr/local/include/ai/
    /usr/local/include/
    ${CMAKE_SOURCE_DIR}/dependencies/arnold/include/
)

FIND_LIBRARY(AI_LIBRARY
    NAMES
    ai
    HINTS
    /home/fabi/Downloads/Arnold-5.4/Arnold-5.4.0.2-linux/bin
    /usr/local/lib/
    /usr/lib/
    ${CMAKE_SOURCE_DIR}/dependencies/arnold/lib/
)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ai DEFAULT_MSG AI_INCLUDE_DIR AI_LIBRARY)
MARK_AS_ADVANCED(AI_INCLUDE_DIR AI_LIBRARY)

IF(AI_FOUND)
    SET(AI_LIBRARIES ${AI_LIBRARY})
    SET(AI_INCLUDE_DIRS ${AI_INCLUDE_DIR})
ENDIF(AI_FOUND)
