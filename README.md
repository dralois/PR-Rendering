# PRRendering

## Configuration & Options
- The config.json file contains options & settings
- The paths to the bodies & scenes need to be set to the respective folders
- The output paths need to be set to some folder
- The objects array should contain all objects that can be rendered

## Required Libraries & Setup
Most of the required libraries are automatically downloaded, compiled and installed correctly, however the requirements listed bellow need to be installed manually. On Linux, Clang-9 is needed to compile PhysX. For everything else GCC-9 is fine. On Windows, Visual Studio 19 has been tested and works, although earlier versions should be fine, too.

### [CMake 3.17](https://cmake.org/download/)
- Download & install at least version 3.15, preferrably 3.17
- Although 3.15 is supported, only 3.17 has been tested

### [Python 3.7](https://www.python.org/downloads/release/python-377/)
- Download & install according to the docs

### [Blender 2.82 as Python Module](https://wiki.blender.org/wiki/Building_Blender)
- Setup the build pipeline accordingly for [Windows](https://wiki.blender.org/wiki/Building_Blender/Windows) or [Linux](https://wiki.blender.org/wiki/Building_Blender/Linux)
- Blender has to be built as described [here](https://wiki.blender.org/wiki/Building_Blender/Other/BlenderAsPyModule), since there is no official release for the python module version
- An unofficial prebuilt version is provided & installed automatically

### [Blenderseed 2.0](https://github.com/appleseedhq/blenderseed/releases)
- Blenderseed just needs to be downloaded, as it is a Blender plugin 
- Linux is currently not offically supported and needs to be built manually
- Appleseed is included in the Blenderseed plugin
- An unofficial prebuilt version is provided & installed automatically

### [Boost 1.73](https://www.boost.org/)
- Most Linux distributions include boost
- On Windows it is necessary to install / build boost
- Boost is statically linked, build / install accordingly
- The required modules are **System, Thread, Filesystem, Python (3.7, may have to be built manually!)**

### [3D Scan Dataset](https://waldjohannau.github.io/RIO/)
- Download script is also provided in data/3Rscan. It requires python2 to run smoothly
- To prepare the dataset, run the _unpack.<span></span>sh_ script and set path to the downloaded data
- Set path of 3RScan in the config.json

### [3D Objects](https://www.alexanderepple.de/pr-rendering-objects-download/)
- The 3D objects can be downloaded through the provided link
- Unpack the zip file into a folder and setup the config file
- Other meshes may be used instead, supported are _Wavefront_ and _glTF_.

## Building

### Windows
- Create a new folder (e.g. _build_) and setup cmake
- Click generate & wait until all dependencies have been built
- Build the project using Visual Studio

### Linux
- Make sure both Clang 9 & GCC 9 are installed
- Make sure boost & boost::python37 are available
- Make sure python 3.7 dev is available
- Create a new folder (e.g. _build_)
- Generate the project from root directory
```shell
cmake -S ./ -B ./build -DCMAKE_BUILD_TYPE=Release
```
- Compile the project in _build_ folder
```shell
cd build && make -j 16
```
