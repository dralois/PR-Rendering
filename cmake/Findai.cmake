# Locates the ai library and include directories.

include(FindPackageHandleStandardArgs)
unset(AI_FOUND)

find_path(ai_INCLUDE_DIR
        NAMES
        ai.h
        HINTS
        # /home/fabi/Downloads/Arnold-5.4/Arnold-5.4.0.2-linux/include)
        /usr/include/
        /usr/local/include/ai/
        /usr/local/include/)

find_library(ai_LIBRARY NAMES ai
        HINTS
        # /home/fabi/Downloads/Arnold-5.4/Arnold-5.4.0.2-linux/bin)
        /usr/local/lib/
        /usr/lib/)

find_package_handle_standard_args(ai DEFAULT_MSG ai_INCLUDE_DIR ai_LIBRARY)

# set external variables for usage in CMakeLists.txt
if(AI_FOUND)
    set(ai_LIBRARIES ${ai_LIBRARY})
    set(ai_INCLUDE_DIRS ${ai_INCLUDE_DIR})
endif()

# hide locals from GUI
mark_as_advanced(ai_INCLUDE_DIR ai_LIBRARY)
