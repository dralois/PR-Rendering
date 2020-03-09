# PRRendering

# Config
- All required paths need to be set in the config.json file

# Required Libraries & Setup (Linux)

## [Arnold Renderer 4.2](https://www.arnoldrenderer.com/arnold/download/archive/arnold-sdk/4.2.9.0/)
- Add the contents of the include directory to the gcc default include directory

    `echo | gcc -Wp,-v -x c++ - -fsyntax-only` can be used to list the default include directories
- And the same for the contents of the bin directory to be added to the gcc default linker search path

    `ldconfig -v 2>/dev/null | grep -v ^$'\t'` can be used to list the default directories

## [OpenCV 4.2](https://opencv.org/releases/) & [RapidJSON 1.1](https://github.com/Tencent/rapidjson/releases)
- OpenCV comes with CMakeLists.txt, so installation should be straight forward
- RapidJSON is header only and comes with CMakeLists.tx, so it should be straight forward as well

## [PhysX 4.1](https://github.com/NVIDIAGameWorks/PhysX)
- Follow [Physx ReadMe](https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/platformreadme/linux/readme_linux.html) to build the SDK (_"it should be the same for mac"_)
- Add the contents of `pxshared/include` and `physx/include` to the gcc default include directory
- Add the compiled files (e.g. `physx/bin/linux.clang/release` content) to the gcc default linker search path

## [Assimp 4.1](http://www.assimp.org/index.php/downloads/) & [Eigen 3.3](http://eigen.tuxfamily.org/index.php?title=Main_Page)
- Assimp has a CMakeLists.txt file so installation is straight forward using cmake
- Eigen has a CMakeLists.txt as well and is header only

## [glm](https://glm.g-truc.net/0.9.9/index.html), [GLFW3](https://www.glfw.org/download.html), [GLEW](http://glew.sourceforge.net/)
- glm is header only and comes with a CMakeLists.txt
- GLFW provides a CMakeLists.txt
- GLEW uses a makefile, so installation should be rather easy

## [Boost 1.72](https://www.boost.org/)
- Most popular Linux distributions come with boost already installed
- Otherwise download and build boost, only filesystem is required

# [Windows Setup & Download](https://www.alexanderepple.de/prrendering-dependencies-download/)
- Windows setup is straightforward, just download and copy the dependencies folder into the source folder
- Use CMake to generate the solution
- Copy the dlls from the copy folder into the respective output directory

## 3D Scan
- Please Download the dataset [here](https://waldjohannau.github.io/RIO/).
- Download script is also provided in data/3Rscan. It requires python2 to run smoothly.
- To prepare the dataset, run the "unpack.sh" script and set path to the downloaded data.
- Set path of 3RScan in the config.json
