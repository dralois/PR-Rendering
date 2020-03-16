# CMake version
CMAKE_MINIMUM_REQUIRED (VERSION 3.15)
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
    SET(CMAKE_VS_SDK_INCLUDE_DIRECTORIES
        "${PROJECT_SOURCE_DIR}/dependencies/include;"
        "${PROJECT_SOURCE_DIR}/dependencies/extra/physx;"
        "${PROJECT_SOURCE_DIR}/dependencies/extra/pxshared;"
        "${PROJECT_SOURCE_DIR}/dependencies/extra/arnold;"
        "$(VC_IncludePath);$(WindowsSDK_IncludePath);")
ENDIF(MSVC)

# Add source files
ADD_EXECUTABLE(PRRendering
                main.cpp
                src/Meshbase.cpp
                src/AiMesh.cpp
                src/PxMesh.cpp
                src/PxMeshConvex.cpp
                src/PxMeshTriangle.cpp
                src/SceneManager.cpp
                src/SimManager.cpp
                include/MeshBase.h
                include/AiMesh.h
                include/PxMesh.h
                include/PxMeshConvex.h
                include/PxMeshTriangle.h
                include/SceneManager.h
                include/SimManager.h
)

###########################################################################################
# Find and link necessary packages
###########################################################################################

# Packages
FIND_PACKAGE(Assimp 4.1 REQUIRED)
FIND_PACKAGE(Eigen3 3.3 REQUIRED)
FIND_PACKAGE(AI 4.2 REQUIRED)
FIND_PACKAGE(OpenCV 4.2 REQUIRED core highgui imgproc objdetect)
FIND_PACKAGE(RapidJSON 1.1 REQUIRED)
FIND_PACKAGE(glm REQUIRED)
FIND_PACKAGE(GLFW3 REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)

# Defines for boost and glew on windows
IF(WIN32)
    SET(BOOST_INCLUDEDIR ${PROJECT_SOURCE_DIR}/dependencies/include)
    SET(BOOST_LIBRARYDIR ${PROJECT_SOURCE_DIR}/dependencies/lib)
    SET(Boost_NAMESPACE "libboost")
ENDIF(WIN32)

# FindBoost is shipped with CMake
FIND_PACKAGE(Boost REQUIRED system thread filesystem)

# Include dirs
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS}
                    ${GLEW_INCLUDE_DIRS}
                    ${GLFW_INCLUDE_DIRS}
                    ${glm_INCLUDE_DIRS}
                    ${RAPIDJSON_INCLUDE_DIRS}
                    ${OPENCV2_INCLUDE_DIRS}
                    ${AI_INCLUDE_DIRS}
                    ${Assimp_INCLUDE_DIRS}
                    ${EIGEN3_INCLUDE_DIRS}
)

# Link to executable
TARGET_LINK_LIBRARIES(PRRendering
                    ${Boost_LIBRARIES}
                    ${GLEW_LIBRARIES}
                    ${GLFW_LIBRARIES}
                    ${glm_LIBRARIES}
                    ${OPENCV2_LIBRARIES}
                    ${AI_LIBRARIES}
                    ${Assimp_LIBRARIES}
)

###########################################################################################
# Shader Library
###########################################################################################

# Create shader library
ADD_LIBRARY(ShadersLib SHARED
            plugins/src/nodeLoader.cpp
            plugins/src/blendShader.cpp
            plugins/src/depthShader.cpp
            plugins/src/labelShader.cpp
            plugins/src/nullFilter.cpp
)

# Link to shader library
TARGET_LINK_LIBRARIES(ShadersLib ${OPENCV2_LIBRARIES} ${AI_LIBRARIES})

###########################################################################################
# OpenGL library
###########################################################################################

# Create OpenGL library
ADD_LIBRARY(OpenGLLib SHARED
            render/include/data.h
            render/include/mesh.h
            render/include/model.h
            render/include/render.h
            render/include/shader.h
            render/include/types.h
            render/include/util.h
            render/src/data.cpp
            render/src/render.cpp
)

# Link to OpenGL library
IF(WIN32)
    TARGET_LINK_LIBRARIES(OpenGLLib ${OPENCV2_LIBRARIES} ${glm_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${Assimp_LIBRARIES} opengl32)
ELSE(WIN32)
    TARGET_LINK_LIBRARIES(OpenGLLib ${OPENGL_LIBRARIES} ${OPENCV2_LIBRARIES} ${glm_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} ${Assimp_LIBRARIES})
ENDIF(WIN32)

###########################################################################################
# Finally setup PhysX
###########################################################################################

FIND_PACKAGE(PhysX 4.0 REQUIRED Common Cooking Extensions Foundation PvdSDK)
# Include dirs
INCLUDE_DIRECTORIES(${PhysX_INCLUDE_DIRS})
# pthread etc. only on Linux
IF(WIN32)
    TARGET_LINK_LIBRARIES(PRRendering ${PhysX_LIBRARIES})
ELSE(WIN32)
    TARGET_LINK_LIBRARIES(PRRendering ${PhysX_LIBRARIES} pthread dl -lboost_system -lboost_thread -lboost_filesystem -Wl,-rpath='${ORIGIN}')
ENDIF(WIN32)

###########################################################################################
# Combine and add libs
###########################################################################################

ADD_LIBRARY(combinedLib INTERFACE)
# Combine into one as interface
TARGET_LINK_LIBRARIES(combinedLib INTERFACE OpenGLLib ShadersLib)
# Add to executable
TARGET_LINK_LIBRARIES(PRRendering combinedLib)

###########################################################################################
# Debug
###########################################################################################

# Print included directories
GET_PROPERTY(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
FOREACH(dir ${dirs})
    MESSAGE(STATUS "dir='${dir}'")
ENDFOREACH()
