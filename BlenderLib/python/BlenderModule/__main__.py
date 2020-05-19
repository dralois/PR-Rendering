import os
import bpy
import mathutils

from . import *

# Init
scene = Managers.SceneManager.Scene()

# Setup test scene
cam = scene.AddCamera("cam")
light = scene.LightManager.CreateLight("light", "POINT")
cube = scene.MeshManager.CreateMesh("cube", None, None)

# Position & adjust objects in scene
light.LightIntensity = 100
light.ObjectPosition = mathutils.Vector((3, -4.2, 5))
scene.Camera.ObjectPosition = mathutils.Vector((6, -3, 5))
scene.Camera.ObjectRotationEuler = mathutils.Euler((0.9, 0.0, 1.1))

# Render camera
scene.Output = os.getcwd() + "//output.png"
scene.Render()
