# PRRendering Dataset Generator

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
- Copy all PhysX libraries & the appleseed library from the _build_ folder to /lib/

## Configuration & Options
- The config.json file contains options & settings
- The paths in the first block need to be set to folders & files
- Simulation & render output can be controlled in the second block
- Object physics can be adjusted in the third block
- Optionally, custom intrinsics can be set in the fourth block
- Lastly, the objects that will be used in the simulation have to be defined

## Scene & Meshes Layout

### Meshes
- Meshes need to be either of _.obj_ or _glTF_ format
- Used meshes need to be listed in the corresponding array in the config file (_render\_objs_)
- Each mesh has three properties but only the path is required
    - Each file path (_mesh\_path_) can be either relative to the base directory (_meshes\_path_) or an absolute path
    - The classification (_mesh\_class_) can be any arbitrary string
    - The scaling factor (_mesh\_unit_) needs to be set so 1 unit = 1 meter
- If available, the diffuse albedo texture needs to be named objectname\__color.png_ and in the same directory as the mesh
- The meshes are read with the assumption that _Y=forward, Z=up_

### Scenes
- Each scene needs to have a _rgbd_ folder and a mesh in _.obj_ format
- The scene mesh needs to be named _mesh.refined.obj_, it may be scaled using the config file (_scene\_unit_)
    - The mesh is read with the assumption that _Y=forward, Z=up_
    - If present, the diffuse albedo texture needs to be named _mesh.refined\_0.png_
- The rgbd folder must contain a _\_info.txt_, containing details about the used camera intrinsics
    - The camera width & height in pixels are named _m\_colorWidth = x_ & _m\_colorHeight = x_
    - The intrinsics are a 4x4, rhs matrix named _m\_calibrationColorIntrinsic = x x ... (16 values)_
    - Alternatively, the intrinsics may be set manually in the config file instead
- The frames have to end with framenum\._color.jpg_ with corresponding framenum\._pose.txt_
- The pose file has to contain only the 4x4, rhs extrinsics matrix, one row per line, with _Y=forward, Z=up_
