function(AddOpenCV TO_TARGET INSTALL_PATH)
    # For reusability
    set(CONTENT_NAME opencv)

    # Parse components
    if(${ARGC} GREATER 2)
        foreach(CURR_COMP ${ARGN})
            set(COMPONENTS ${COMPONENTS} ${CURR_COMP})
        endforeach()
    endif()

    # Check if package with components available
    CheckOpenCV(CHECK_FOUND ${COMPONENTS})

    # Load and build if not so
    if(NOT ${CHECK_FOUND})
        # Enable dependency download module
        include(FetchContent)
        include(ContentHelpers)
        # Download source code
        FetchContent_Declare(${CONTENT_NAME}
                            GIT_REPOSITORY https://github.com/opencv/opencv.git
                            GIT_TAG 4.3.0
                            GIT_SHALLOW True
                            GIT_PROGRESS True
        )
        # Make available
        FetchContent_GetProperties(${CONTENT_NAME})
        if(NOT ${CONTENT_NAME}_POPULATED)
            FetchContent_Populate(${CONTENT_NAME})
            # Configure opencv
            CreateContent(${${CONTENT_NAME}_SOURCE_DIR} ${${CONTENT_NAME}_BINARY_DIR}
                        CMAKE_INSTALL_PREFIX=${INSTALL_PATH}
                        WITH_OPENCL=OFF
                        WITH_QUIRC=OFF
                        WITH_OPENCL_D3D11_NV=OFF
                        WITH_OPENCLAMDBLAS=OFF
                        WITH_OPENCLAMDFFT=OFF
                        WITH_WEBP=OFF
                        WITH_GSTREAMER=OFF
                        BUILD_opencv_apps=OFF
                        BUILD_opencv_features2d=ON
                        BUILD_opencv_python_bindings_generator=OFF
                        BUILD_opencv_photo=OFF
                        BUILD_opencv_world=ON
                        BUILD_opencv_python_tests=OFF
                        BUILD_opencv_ml=OFF
                        BUILD_opencv_dnn=OFF
                        BUILD_opencv_ts=OFF
                        BUILD_opencv_gapi=OFF
                        BUILD_opencv_flann=ON
                        BUILD_opencv_video=OFF
                        BUILD_opencv_imgcodecs=ON
                        BUILD_opencv_calib3d=ON
                        BUILD_opencv_videoio=OFF
                        BUILD_opencv_stitching=OFF
                        BUILD_opencv_java_bindings_generator=OFF
                        BUILD_PERF_TESTS=OFF
                        BUILD_PACKAGE=OFF
                        BUILD_WEBP=OFF
                        BUILD_TESTS=OFF
                        BUILD_JAVA=OFF
                        CPACK_SOURCE_7Z=OFF
                        CPACK_SOURCE_ZIP=OFF
                        CPACK_BINARY_NSIS=OFF
                        OPENCV_DNN_OPENCL=OFF
            )
            # Build opencv
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "release")
            BuildContent(${${CONTENT_NAME}_BINARY_DIR} "debug")
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "release" ${INSTALL_PATH})
            InstallContent(${${CONTENT_NAME}_BINARY_DIR} "debug" ${INSTALL_PATH})
        endif()
        # Load package with components
        CheckOpenCV(CHECK_FOUND ${COMPONENTS})
    endif()

    # Copy required dlls
    if(WIN32)
        CopyContent(${TO_TARGET} ${INSTALL_PATH}/x64/vc16/bin ${INSTALL_PATH}/x64/vc16/bin true)
    endif()

    # Link and include components
    target_link_libraries(${TO_TARGET} PRIVATE ${COMPONENTS})
endfunction()

function(CheckOpenCV CHECK_FOUND)
    # Search for components or in general
    if(${ARGC} GREATER 1)
        find_package(OpenCV
                    COMPONENTS
                    ${ARGN}
                    PATHS
                    ${INSTALL_PATH}
                    NO_DEFAULT_PATH
        )
    else()
        find_package(OpenCV
                    PATHS
                    ${INSTALL_PATH}
                    NO_DEFAULT_PATH
        )
    endif()
    # Return result
    set(CHECK_FOUND ${OpenCV_FOUND} PARENT_SCOPE)
endfunction()
