# CMake version
cmake_minimum_required (VERSION 3.11)
# Project name
project (PRRendering CXX)

###########################################################################################

# CMake paths
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/../Dependencies/eigen3-3.3.7/cmake)

# C++ 14 standards
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Headers
include_directories("${PROJECT_SOURCE_DIR}/include")

# Source files
add_executable(PRRendering src/main src/mesh_managers.cpp src/scene_manager.cpp src/sim_manager.cpp)
# Shader library
add_library(Plugins SHARED Plugins/src/nodeLoader.cpp Plugins/src/blendShader.cpp Plugins/src/depthShader.cpp Plugins/src/labelShader.cpp Plugins/src/nullFilter.cpp)

# Find Assimp
find_package(assimp 4.1 REQUIRED)
# Link if found
if(NOT ${ASSIMP_FOUND})
    message("Assimp (min. 4.1) not found!")
else()
    target_link_libraries(PRRendering ${ASSIMP_LIBRARIES})
    include_directories(${ASSIMP_INCLUDE_DIRS})
endif()

# Find Eigen
find_package(Eigen3 3.3 REQUIRED)
# Link if found
if(NOT ${EIGEN3_FOUND})
    message("Eigen3 (min. 3.3) not found!")
else()
    include_directories(${EIGEN3_INCLUDE_DIRS})
endif()

# Find Arnold SDK
find_package(ai 5.4 REQUIRED)
# Link if found
if(NOT ${AI_FOUND})
    message("Arnold SDK (min. 5.4) not found!")
else()
    target_link_libraries (PRRendering ${AI_LIBRARIES})
    include_directories(${AI_INCLUDE_DIRS})
endif()

# Find OpenCV
find_package(OpenCV 4.2 REQUIRED core highgui imgproc objdetect)
# Link if found
if(NOT ${OPENCV_FOUND})
    message("OpenCV (min. 4.2) not found!")
else()
    target_link_libraries (PRRendering ${OPENCV_LIBRARIES})
    include_directories(${OPENCV_INCLUDE_DIRS})
endif()

# Find RapidJSON
find_package(RapidJSON 1.1 REQUIRED)
# Link if found
if(NOT ${RAPIDJSON_FOUND})
    message("RapidJSON (min 1.1) not found!")
else()
    include_directories(${RAPIDJSON_INCLUDE_DIRS})
endif()

# Find PhysX
find_package(PhysX 4.1 REQUIRED Common Cooking Extensions Foundation PvdSDK)
# Link if found
if(NOT ${PHYSX_FOUND})
    message("PhysX (min. 4.1) not found!")
else()
    target_link_libraries (PRRendering ${PhysX_LIBRARIES})
    #target_link_libraries (PRRendering ${PhysX_LIBRARIES} ${Plugins} pthread dl -Wl,-rpath='${ORIGIN}')
    include_directories(${PhysX_INCLUDE_DIRS})
endif()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "dir='${dir}'")
endforeach()
