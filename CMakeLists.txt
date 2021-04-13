# Check cmake version
cmake_minimum_required(VERSION 3.15...3.19)

###########################################################################################
# Setup project
###########################################################################################

# Create project
project(PRRendering
        VERSION 1.3
        DESCRIPTION "PR Rendering"
        LANGUAGES CXX)

# Append module paths
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# C++ 14 standards
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON)

# Enable folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

###########################################################################################
# Main executable
###########################################################################################

# Generate code file lists
file(GLOB_RECURSE HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SOURCE_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Set IDE folder structure
source_group(TREE "${CMAKE_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})
source_group(TREE "${CMAKE_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${SOURCE_LIST})
source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${CMAKE_SOURCE_DIR}/main.cpp)

# Create project executable
add_executable(PRRendering ${HEADER_LIST} ${SOURCE_LIST} ${CMAKE_SOURCE_DIR}/main.cpp)
if(WIN32 AND MSVC)
    file(RELATIVE_PATH CONFIG_PATH ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/config_alex.json)
    set_target_properties(PRRendering PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${CONFIG_PATH}")
endif()

# Make includes available
target_include_directories(PRRendering PRIVATE include)

# Configure output
set_target_properties(PRRendering PROPERTIES
                    PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
                    COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
                    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
                    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
)

###########################################################################################
# External packages
###########################################################################################

# Find & add boost
set(Boost_ADDITIONAL_VERSIONS "1.73.0")
find_package(Boost REQUIRED system thread filesystem)
target_link_libraries(PRRendering PRIVATE Boost::system Boost::thread Boost::filesystem)

# Set external path
set(PROJECT_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

# Include cmake files
include(AddEigen)
include(AddAssimp)
include(AddOpenCV)
include(AddPhysX)
include(AddRapidJSON)

# Add dependencies
AddEigen(PRRendering ${PROJECT_EXTERNAL_DIR}/eigen)
AddAssimp(PRRendering ${PROJECT_EXTERNAL_DIR}/assimp)
AddOpenCV(PRRendering ${PROJECT_EXTERNAL_DIR}/opencv
    opencv_core opencv_highgui opencv_imgproc opencv_objdetect)
AddPhysX(PRRendering ${PROJECT_EXTERNAL_DIR}/physx
    PhysX::PhysX PhysX::PhysXPvdSDK PhysX::PhysXExtensions PhysX::PhysXCooking PhysX::PhysXCommon PhysX::PhysXFoundation)
AddRapidJSON(PRRendering ${PROJECT_EXTERNAL_DIR}/rapidjson)

###########################################################################################
# Additional libraries
###########################################################################################

add_subdirectory(${CMAKE_SOURCE_DIR}/BlenderLib)
target_link_libraries(PRRendering PRIVATE BlenderLib)
