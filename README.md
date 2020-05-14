# PRRendering

# Config
- All required paths need to be set in the config.json file

# Required Libraries & Setup
Most of the required libraries are automatically downloaded, compiled and installed correctly, however the requirements listed bellow need to be installed manually.

## [Python 3.7](https://www.python.org/downloads/release/python-377/)
- Download & install according to the docs

## [Blender 2.82 as Python Module](https://wiki.blender.org/wiki/Building_Blender/Other/BlenderAsPyModule)
- Blender has to be built as described [here](https://wiki.blender.org/wiki/Building_Blender/Other/BlenderAsPyModule), since there is no official release for the python module version
- This will be done automatically in the future

## [Blenderseed 2.0](https://github.com/appleseedhq/blenderseed/releases)
- Blenderseed just needs to be downloaded, as it is a Blender plugin 
- Linux is currently not offically supported and needs to be built manually
- Appleseed is included in the Blenderseed plugin
- This will be done automatically in the future

## [Boost 1.73](https://www.boost.org/)
- Most Linux distributions include boost
- On Windows it is necessary to install / build boost
- Boost is statically linked, build / install accordingly
- The required modules are **System, Thread, Filesystem, Python (3.7!)**

## [3D Scan Dataset](https://waldjohannau.github.io/RIO/)
- Download script is also provided in data/3Rscan. It requires python2 to run smoothly
- To prepare the dataset, run the "unpack.sh" script and set path to the downloaded data
- Set path of 3RScan in the config.json
