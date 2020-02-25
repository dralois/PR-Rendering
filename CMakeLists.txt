# CMake version
CMAKE_MINIMUM_REQUIRED (VERSION 3.11)
# Project name
PROJECT (PRRendering CXX)

###########################################################################################

# CMake paths
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/../Dependencies/eigen3-3.3.7/cmake)

# C++ 14 standards
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Headers
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/include")

# Dirent.h for windows
IF(MSVC)
    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/dependencies/dirent/include)
ENDIF()

# Source files
ADD_EXECUTABLE(PRRendering src/main src/mesh_managers.cpp src/scene_manager.cpp src/sim_manager.cpp)
# Shader library
ADD_LIBRARY(Plugins SHARED Plugins/src/nodeLoader.cpp Plugins/src/blendShader.cpp Plugins/src/depthShader.cpp Plugins/src/labelShader.cpp Plugins/src/nullFilter.cpp)

# Find Assimp
FIND_PACKAGE(assimp 4.1 REQUIRED)
# Link if found
IF(NOT ${ASSIMP_FOUND})
    MESSAGE("Assimp (min. 4.1) not found!")
ELSE()
    TARGET_LINK_LIBRARIES(PRRendering ${ASSIMP_LIBRARIES})
    INCLUDE_DIRECTORIES(${ASSIMP_INCLUDE_DIRS})
ENDIF()

# Find Eigen
FIND_PACKAGE(Eigen3 3.3 REQUIRED)
# Link if found
IF(NOT ${EIGEN3_FOUND})
    MESSAGE("Eigen3 (min. 3.3) not found!")
ELSE()
    INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIRS})
ENDIF()

# Find Arnold SDK
FIND_PACKAGE(ai 5.4 REQUIRED)
# Link if found
IF(NOT ${AI_FOUND})
    MESSAGE("Arnold SDK (min. 5.4) not found!")
ELSE()
    TARGET_LINK_LIBRARIES(PRRendering ${AI_LIBRARIES})
    INCLUDE_DIRECTORIES(${AI_INCLUDE_DIRS})
ENDIF()

# Find OpenCV
FIND_PACKAGE(OpenCV 4.2 REQUIRED core highgui imgproc objdetect)
# Link if found
IF(NOT ${OPENCV2_FOUND})
    MESSAGE("OpenCV (min. 4.2) not found!")
ELSE()
    TARGET_LINK_LIBRARIES(PRRendering ${OPENCV2_LIBRARIES})
    INCLUDE_DIRECTORIES(${OPENCV2_INCLUDE_DIRS})
ENDIF()

# Find RapidJSON
FIND_PACKAGE(RapidJSON 1.1 REQUIRED)
# Link if found
IF(NOT ${RAPIDJSON_FOUND})
    MESSAGE("RapidJSON (min 1.1) not found!")
ELSE()
    INCLUDE_DIRECTORIES(${RAPIDJSON_INCLUDE_DIRS})
ENDIF()

# Find PhysX
FIND_PACKAGE(PhysX 4.1 REQUIRED Common Cooking Extensions Foundation PvdSDK)
# Link if found
IF(NOT ${PHYSX_FOUND})
    MESSAGE("PhysX (min. 4.1) not found!")
ELSE()
    TARGET_LINK_LIBRARIES(PRRendering ${PhysX_LIBRARIES})
    #TARGET_LINK_LIBRARIES (PRRendering ${PhysX_LIBRARIES} ${Plugins} pthread dl -Wl,-rpath='${ORIGIN}')
    INCLUDE_DIRECTORIES(${PhysX_INCLUDE_DIRS})
ENDIF()

# Print included directories
GET_PROPERTY(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
FOREACH(dir ${dirs})
    MESSAGE(STATUS "dir='${dir}'")
ENDFOREACH()
