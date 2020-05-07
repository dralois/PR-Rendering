# Check cmake version
cmake_minimum_required(VERSION 3.15...3.18)

###########################################################################################
# Setup project
###########################################################################################

# Create project
project(BlenderLib
        VERSION 1.0
        DESCRIPTION "Blenderseed for PR Rendering"
        LANGUAGES CXX
)

# C++ 14 standards
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# Enable nice folder structure
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

###########################################################################################
# Main library
###########################################################################################

# Generate code file lists
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB PYTHON_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/python/*.py")

# Set IDE folder structure
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${SOURCE_LIST})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/python" PREFIX "Python Files" FILES ${PYTHON_LIST})

# Create library
add_library(BlenderLib SHARED ${HEADER_LIST} ${SOURCE_LIST} ${PYTHON_LIST})

# Make includes available
target_include_directories(BlenderLib PUBLIC include)

###########################################################################################
# External packages
###########################################################################################

# Disable python debug libraries
set(Boost_USE_DEBUG_PYTHON OFF)

# Find & add python
find_package(Python 3.7 REQUIRED Development)
target_link_libraries(BlenderLib PRIVATE Python::Python)

# Find & add boost
find_package(Boost REQUIRED thread python37)
target_link_libraries(BlenderLib PRIVATE Boost::thread Boost::python37)

# TODO: Find & add Blender
# TODO: Find & add Blenderseed

# Finally add linker flags on Linux and dll copy on Windows
if(UNIX)
    target_link_options(BlenderLib PRIVATE pthread dl -lboost_system -lboost_thread -lboost_python37)
else()
    include(ContentHelpers)
    # Add copy target
    add_custom_target(CopyDlls${PROJECT_NAME} COMMENT "Copies required dlls" VERBATIM)
    # Add copy commands
    CopyContent(CopyDlls${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/Debug ${CMAKE_CURRENT_BINARY_DIR}/Release)
    # Make library depend on it
    add_dependencies(BlenderLib CopyDlls${PROJECT_NAME})
    # Hide in IDE folder
    set_target_properties(CopyDlls${PROJECT_NAME} PROPERTIES FOLDER "DLL Copy")
endif()
