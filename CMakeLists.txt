cmake_minimum_required (VERSION 3.11)
project (PRRendering)

###########################################################################################
# Find modules
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(PRRendering src/main src/mesh_managers.cpp src/scene_manager.cpp src/sim_manager.cpp)
add_library(Plugins SHARED Plugins/src/nodeLoader.cpp Plugins/src/blendShader.cpp Plugins/src/depthShader.cpp Plugins/src/labelShader.cpp Plugins/src/nullFilter.cpp)

find_package(assimp REQUIRED)
target_link_libraries(PRRendering assimp)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

find_package (ai REQUIRED)
target_link_libraries (PRRendering ${ai_LIBRARIES})
include_directories(${ai_INCLUDE_DIRS})

find_package(OpenCV 3.0 REQUIRED)
target_link_libraries (PRRendering ${OpenCV_LIBS})
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(RapidJSON REQUIRED)

if(NOT DEFINED ENV{PHYSX_ROOT_DIR})
    message("Could not find PhysX")
    set(ENV{PHYSX_ROOT_DIR} "/home/fabi/Documents/PhysX-4.0.0/physx")
endif()

message("phyx: $ENV{PHYSX_ROOT_DIR}")

set(PHYSX_BIN "$ENV{PHYSX_ROOT_DIR}/bin/linux.clang/release")

set(PHYSX_LIBRARIES ${PHYSX_BIN}/libPhysX_static_64.a ${PHYSX_BIN}/libPhysXPvdSDK_static_64.a ${PHYSX_BIN}/libPhysXExtensions_static_64.a ${PHYSX_BIN}/libPhysXCooking_static_64.a ${PHYSX_BIN}/libPhysXCommon_static_64.a ${PHYSX_BIN}/libPhysXFoundation_static_64.a)
include_directories("$ENV{PHYSX_ROOT_DIR}/install/linux/PhysX/include")
include_directories("$ENV{PHYSX_ROOT_DIR}/install/linux/PxShared/include")
include_directories("$ENV{PHYSX_ROOT_DIR}/include/")

link_directories("$ENV{PHYSX_ROOT_DIR}/bin/linux.clang/release")
target_link_libraries (PRRendering  ${PHYSX_LIBRARIES} ${Plugins} pthread dl -Wl,-rpath='${ORIGIN}')
