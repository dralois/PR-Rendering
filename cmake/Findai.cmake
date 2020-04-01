# - Find Arnold SDK
# Find the Arnold SDK includes and library
#
#  AI_INCLUDE_DIRS - Where to find Arnold SDK includes
#  AI_LIBRARIES    - List of libraries when using Arnold SDK
#  AI_FOUND        - True if Arnold SDK was found

IF(AI_INCLUDE_DIR)
  SET(AI_FIND_QUIETLY TRUE)
ENDIF(AI_INCLUDE_DIR)

IF(WIN32)
    FIND_PATH(AI_INCLUDE_DIR
            NAMES
            ai.h
            HINTS
            $ENV{PROGRAMFILES}/Arnold/include
            ${CMAKE_SOURCE_DIR}/dependencies/extra/arnold/
    )
    FIND_LIBRARY(AI_LIBRARY
                NAMES
                ai
                HINTS
                $ENV{PROGRAMFILES}/Arnold/lib
                ${CMAKE_SOURCE_DIR}/dependencies/lib/
    )
ELSE(WIN32)
    FIND_PATH(AI_INCLUDE_DIR
            NAMES
            ai.h
            HINTS
            /sw/include
            /usr/include
            /usr/local/include
            /opt/local/include
            /usr/local/include/ai
            /home/fabi/Downloads/Arnold-6.0/Arnold-6.0.2.0-linux/include
            /home/fabi/Downloads/Arnold-5.4/Arnold-5.4.0.2-linux/include
            /home/fabi/Downloads/Arnold-4.2/Arnold-4.2.9.0-linux/include
    )
    FIND_LIBRARY(AI_LIBRARY
                NAMES
                ai
                HINTS
                /sw/lib
                /usr/lib
                /usr/lib64
                /opt/local/lib
                /usr/local/lib
                /usr/local/lib64
                /home/fabi/Downloads/Arnold-6.0/Arnold-6.0.2.0-linux/bin
                /home/fabi/Downloads/Arnold-5.4/Arnold-5.4.0.2-linux/bin
                /home/fabi/Downloads/Arnold-4.2/Arnold-4.2.9.0-linux/bin
    )
ENDIF(WIN32)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(AI DEFAULT_MSG AI_INCLUDE_DIR AI_LIBRARY)
MARK_AS_ADVANCED(AI_INCLUDE_DIR AI_LIBRARY)

IF(AI_FOUND)
    SET(AI_LIBRARIES ${AI_LIBRARY})
    SET(AI_INCLUDE_DIRS ${AI_INCLUDE_DIR})
ENDIF(AI_FOUND)
