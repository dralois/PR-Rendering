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

# Set external path if not already set
if(NOT EXISTS ${PROJECT_EXTERNAL_DIR})
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)
    set(PROJECT_EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external)
endif()

###########################################################################################
# Main library
###########################################################################################

# Generate code file lists
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/BlenderLib/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE PYTHON_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/python/*.py")
file(GLOB_RECURSE SHADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/python/BlenderModule/*.osl")
list(FILTER PYTHON_LIST EXCLUDE REGEX ".*blenderseed.*")

# Set IDE folder structure
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/BlenderLib" PREFIX "Header Files" FILES ${HEADER_LIST})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${SOURCE_LIST})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/python" PREFIX "Python Files" FILES ${PYTHON_LIST})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/python/BlenderModule" PREFIX "Shader Files" FILES ${SHADER_LIST})

# Create library
add_library(BlenderLib SHARED ${HEADER_LIST} ${SOURCE_LIST} ${PYTHON_LIST} ${SHADER_LIST})

# Make includes available
target_include_directories(BlenderLib INTERFACE include)
target_include_directories(BlenderLib PRIVATE include/BlenderLib)

# Configure output
set_target_properties(BlenderLib PROPERTIES
                    PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
                    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
                    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
)

###########################################################################################
# External packages
###########################################################################################

# Disable python debug libraries
set(Boost_ADDITIONAL_VERSIONS "1.73.0")
set(Boost_USE_DEBUG_PYTHON OFF)

# Find python for development
find_package(Python 3.7 REQUIRED Development)
# Bug: Enforce no debug on Windows
if(WIN32)
    set_target_properties(Python::Python PROPERTIES
                        IMPORTED_IMPLIB_DEBUG "${Python_LIBRARY_RELEASE}"
                        IMPORTED_LOCATION_DEBUG "${Python_RUNTIME_LIBRARY_RELEASE}"
    )
endif()
# Add python development library
target_link_libraries(BlenderLib PRIVATE Python::Python)

# Find & add boost
find_package(Boost REQUIRED thread python37)
target_link_libraries(BlenderLib PRIVATE Boost::thread Boost::python37)

# Include cmake modules
include(AddBlender)
include(AddAppleseed)

# TODO
# Add dependencies
# AddBlender(BlenderLib ${PROJECT_EXTERNAL_DIR}/blender)
# AddAppleseed(BlenderLib ${PROJECT_EXTERNAL_DIR}/appleseed)

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
