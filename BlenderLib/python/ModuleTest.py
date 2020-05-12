import os
import bpy
import mathutils

from BlenderModule import Managers

# For testing
if __name__ == "__main__":
    # Init
    Managers.Initialize(os.path.dirname(__file__), os.path.dirname(__file__))
    sceneMgr = Managers.SceneManager.SceneObjects()
    # Setup test scene
    cam = sceneMgr.AddCamera("cam")
    light = sceneMgr.AddLight("light")
    cube = sceneMgr.AddMesh("cube")
    # Create objects
    cam.CreateCamera(0.0, 0.0, 0.0, 0.0)
    light.CreatePointLight(100.0)
    cube.CreateCube(None)
    # Position objects in scene
    light.Position = mathutils.Vector((3, -4.2, 5))
    cam.Position = mathutils.Vector((6, -3, 5))
    cam.RotationEuler = mathutils.Euler((0.9, 0.0, 1.1))
    # Render
    sceneMgr.TryRenderNextCamera()
