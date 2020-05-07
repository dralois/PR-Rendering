# Check cmake version
cmake_minimum_required(VERSION 3.15...3.18)

###########################################################################################
# Setup project
###########################################################################################

# Create project
project(ShadersLib
        VERSION 1.0
        DESCRIPTION "Arnold shaders for PR Rendering"
        LANGUAGES CXX
)

# C++ 14 standards
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Enable nice folder structure
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set external path if not already set
if(NOT EXISTS ${PROJECT_EXTERNAL_DIR})
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
    set(PROJECT_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external)
endif()

###########################################################################################
# Main library
###########################################################################################

# Generate & set IDE folder structure
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Shaders" FILES ${SOURCE_LIST})

# Create library
add_library(ShadersLib SHARED ${SOURCE_LIST})

###########################################################################################
# External packages
###########################################################################################

# Find & add Arnold
find_package(AI REQUIRED MODULE)
target_link_libraries(ShadersLib PRIVATE AI::AI)

# Find & add OpenCV
include(AddOpenCV)
AddOpenCV(ShadersLib ${PROJECT_EXTERNAL_DIR}/opencv
    opencv_core opencv_highgui opencv_imgproc opencv_objdetect)

# Finally add linker flags on Linux and dll copy on Windows
if(UNIX)
    target_link_options(ShadersLib PRIVATE pthread dl)
else()
    include(ContentHelpers)
    # Add copy target
    add_custom_target(CopyDlls${PROJECT_NAME} COMMENT "Copies required dlls" VERBATIM)
    # Add copy commands
    CopyContent(CopyDlls${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/Debug ${CMAKE_CURRENT_BINARY_DIR}/Release)
    # Make library depend on it
    add_dependencies(ShadersLib CopyDlls${PROJECT_NAME})
    # Hide in IDE folder
    set_target_properties(CopyDlls${PROJECT_NAME} PROPERTIES FOLDER "DLL Copy")
endif()
