# PRRendering

# Config
- All required paths need to be set in the config.json file

# Required Libraries

## [Arnold Renderer 4](https://www.arnoldrenderer.com/arnold/download/archive/arnold-sdk/4.2.9.0/)
- Add the contents of the include directory to the gcc default include directory

`echo | gcc -Wp,-v -x c++ - -fsyntax-only` can be used to list the default include directories
- And the same for the contents of the bin directory to be added to the gcc default linker search path

`ldconfig -v 2>/dev/null | grep -v ^$'\t'` can be used to list the default directories
## [Physix-4.1](https://github.com/NVIDIAGameWorks/PhysX)
*I've only struggled running physix when using linux so I'll only consider linux here*

- Follow [Physx ReadMe](https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/platformreadme/linux/readme_linux.html) to build the SDK "it should be the same for mac and windows". 
- Add the contents of `pxshared/include` and `physx/include` to the gcc default include directory.
- Add the compiled files (in my case `physx/bin/linux.clang/release` content) to the gcc default linker search path

## [Assimp-4.1](http://www.assimp.org/index.php/downloads/) & [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
- Assimp and Eigen both have CMakeLists.txt file so installation is straight forward using cmake.

## 3D Scan
- Please Download the dataset [here](https://waldjohannau.github.io/RIO/).
- Download script is also provided in data/3Rscan. It requires python2 to run smoothly.
- To prepare the dataset, run the "unpack.sh" script and set path to the downloaded data.
- Set path of 3RScan in the config.json
