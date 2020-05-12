# Check cmake version
cmake_minimum_required(VERSION 3.15...3.18)

###########################################################################################
# Setup project
###########################################################################################

# Create project
project(OpenGLLib
        VERSION 1.0
        DESCRIPTION "OpenGL Renderer for PR Rendering"
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
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB SHADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.*")

# Set IDE folder structure
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${SOURCE_LIST})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/shaders" PREFIX "Shader Files" FILES ${SHADER_LIST})

# Create library
add_library(OpenGLLib SHARED ${HEADER_LIST} ${SOURCE_LIST} ${SHADER_LIST})

# Make includes available
target_include_directories(OpenGLLib PUBLIC include)

# Configure output
set_target_properties(OpenGLLib PROPERTIES
                    PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
                    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
                    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
)

###########################################################################################
# External packages
###########################################################################################

# Include cmake files
include(AddEigen)
include(AddGLM)
include(AddGLEW)
include(AddGLFW)
include(AddAssimp)
include(AddOpenCV)

# Add dependencies
AddAssimp(OpenGLLib ${PROJECT_EXTERNAL_DIR}/assimp)
AddEigen(OpenGLLib ${PROJECT_EXTERNAL_DIR}/eigen)
AddGLM(OpenGLLib ${PROJECT_EXTERNAL_DIR}/glm)
AddGLEW(OpenGLLib ${PROJECT_EXTERNAL_DIR}/glew)
AddGLFW(OpenGLLib ${PROJECT_EXTERNAL_DIR}/glfw)
AddOpenCV(OpenGLLib ${PROJECT_EXTERNAL_DIR}/opencv
    opencv_core opencv_highgui opencv_imgproc opencv_objdetect)

# Finally add linker flags on Linux and dll copy on Windows
if(UNIX)
    target_link_options(OpenGLLib PRIVATE pthread dl)
else()
    include(ContentHelpers)
    # Add copy target
    add_custom_target(CopyDlls${PROJECT_NAME} COMMENT "Copies required dlls" VERBATIM)
    # Add copy commands
    CopyContent(CopyDlls${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/Debug ${CMAKE_CURRENT_BINARY_DIR}/Release)
    # Make library depend on it
    add_dependencies(OpenGLLib CopyDlls${PROJECT_NAME})
    # Hide in IDE folder
    set_target_properties(CopyDlls${PROJECT_NAME} PROPERTIES FOLDER "DLL Copy")
endif()
