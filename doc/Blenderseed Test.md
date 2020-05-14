# Requirements
- Blender 2.82a as Python module, installed accordingly
- Blenderseed 2.0 downloaded as a zip file
- Linux: Appleseed 2.1 pre-built binaries

# Blender Python Module Test
The following code should work in a python terminal, no matter where it is executed. If this is not the case the next test will not work.
``` Python
import bpy
# Renders the default blender scene to file
bpy.ops.render.render(write_still=True)
```

# Blenderseed Plugin Test
1. Move the script _BlenderLib/python/BlenderTest.py_ to the folder where the _blenderseed.zip_ resides
2. Linux: Extract the zip file, replace the content of the folders _appleseed/bin_ and _appleseed/lib_ with the Linux binaries
3. Linux: Zip the modified content back into _blenderseed.zip_
4. Execute the script using Python 3.7
5. Rendering into a png file should commence
