# CMake version
CMAKE_MINIMUM_REQUIRED (VERSION 3.11)
# Project name
PROJECT (PRRendering CXX)

###########################################################################################
# Setup project
###########################################################################################

# CMake paths
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/../Dependencies/eigen3-3.3.7/cmake)

# C++ 14 standards
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header files
INCLUDE_DIRECTORIES("include")

# Visual Studio only: Change include directory
IF(MSVC)
    INCLUDE_DIRECTORIES(dependencies/include/dirent)
    SET(CMAKE_VS_SDK_INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/dependencies/include;${PROJECT_SOURCE_DIR}/dependencies/extra/physx;${PROJECT_SOURCE_DIR}/dependencies/extra/pxshared;${PROJECT_SOURCE_DIR}/dependencies/extra/arnold;$(VC_IncludePath);$(WindowsSDK_IncludePath);)
ENDIF()

# Add source files
ADD_EXECUTABLE(PRRendering
                main
                src/mesh_managers.cpp
                src/scene_manager.cpp
                src/sim_manager.cpp
                include/mesh_managers.h
                include/scene_manager.h
                include/sim_manager.h
)

# Create shader library
ADD_LIBRARY(Plugins SHARED
            plugins/src/nodeLoader.cpp
            plugins/src/blendShader.cpp
            plugins/src/depthShader.cpp
            plugins/src/labelShader.cpp
            plugins/src/nullFilter.cpp
)

###########################################################################################
# Find and link necessary packages
###########################################################################################

FIND_PACKAGE(Assimp 4.1 REQUIRED)
IF(NOT ${Assimp_FOUND})
    MESSAGE("Assimp (min. 4.1) not found!")
ELSE()
    TARGET_LINK_LIBRARIES(PRRendering ${Assimp_LIBRARIES})
    INCLUDE_DIRECTORIES(${Assimp_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(Eigen3 3.3 REQUIRED)
IF(NOT ${EIGEN3_FOUND})
    MESSAGE("Eigen3 (min. 3.3) not found!")
ELSE()
    INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(AI 5.4 REQUIRED)
IF(NOT ${AI_FOUND})
    MESSAGE("Arnold SDK (min. 5.4) not found!")
ELSE()
    TARGET_LINK_LIBRARIES(PRRendering ${AI_LIBRARIES})
    TARGET_LINK_LIBRARIES(Plugins ${AI_LIBRARIES})
    INCLUDE_DIRECTORIES(${AI_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(OpenCV 4.2 REQUIRED core highgui imgproc objdetect)
IF(NOT ${OPENCV2_FOUND})
    MESSAGE("OpenCV2 (min. 4.2) (core, highgui, imgproc, objdetect) not found!")
ELSE()
    TARGET_LINK_LIBRARIES(PRRendering ${OPENCV2_LIBRARIES})
    INCLUDE_DIRECTORIES(${OPENCV2_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(RapidJSON 1.1 REQUIRED)
IF(NOT ${RAPIDJSON_FOUND})
    MESSAGE("RapidJSON (min 1.1) not found!")
ELSE()
    INCLUDE_DIRECTORIES(${RAPIDJSON_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(glm REQUIRED)
IF(NOT ${glm_FOUND})
    MESSAGE("glm not found!")
ELSE()
    INCLUDE_DIRECTORIES(${glm_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(PRRendering ${glm_LIBRARIES})
ENDIF()

FIND_PACKAGE(GLFW3 REQUIRED)
IF(NOT ${GLFW_FOUND})
    MESSAGE("GLFW3 not found!")
ELSE()
    INCLUDE_DIRECTORIES(${GLFW_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(PRRendering ${GLFW_LIBRARIES})
ENDIF()

FIND_PACKAGE(GLEW REQUIRED)
IF(NOT ${GLEW_FOUND})
    MESSAGE("GLEW not found!")
ELSE()
    INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(PRRendering ${GLEW_LIBRARIES})
ENDIF()

FIND_PACKAGE(Boost REQUIRED)
IF(NOT ${Boost_FOUND})
    MESSAGE("Boost not found!")
ELSE()
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(PRRendering ${Boost_LIBRARIES})
ENDIF()

###########################################################################################
# OpenGL library
###########################################################################################

# Create OpenGL library
ADD_LIBRARY(OpenGLLib SHARED
            render/include/model.h
            render/include/data.h
            render/include/render.h
            render/include/shader.h
            render/include/util.h
            render/src/data.cc
            render/src/render.cc
)

# Link to OpenGL library
TARGET_LINK_LIBRARIES(OpenGLLib ${OPENCV2_LIBRARIES} ${glm_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${Assimp_LIBRARIES})

###########################################################################################
# Setup PhysX
###########################################################################################

FIND_PACKAGE(PhysX 4.1 REQUIRED Common Cooking Extensions Foundation PvdSDK)
IF(NOT ${PHYSX_FOUND})
    MESSAGE("PhysX (min. 4.1) (Common, Cooking, Extensions, Foundation, PvdSDK) not found!")
ELSE()
    TARGET_LINK_LIBRARIES(PRRendering ${PhysX_LIBRARIES})
    # TODO: What is this command doing?
    #TARGET_LINK_LIBRARIES(PRRendering ${PHYSX_LIBRARIES} ${Plugins} ${OpenCV_LIBS} OpenGLLib pthread dl -lboost_system -lboost_thread -lboost_filesystem -Wl,-rpath='${ORIGIN}')
    INCLUDE_DIRECTORIES(${PhysX_INCLUDE_DIRS})
ENDIF()

###########################################################################################
# Debug
###########################################################################################

# Print included directories
GET_PROPERTY(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
FOREACH(dir ${dirs})
    MESSAGE(STATUS "dir='${dir}'")
ENDFOREACH()
