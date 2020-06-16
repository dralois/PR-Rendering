from .Managers.RenderManager import RenderScenes
from .Managers.SceneManager import SceneProxy
from .Utils.BridgeObjects import CXXCamera, CXXLight, CXXMesh, CXXSettings

import os
import mathutils

settings = CXXSettings()
settings.Output = os.path.join(os.getcwd(), "Module Test\\")
settings.Plugin = os.path.join(os.getcwd(), "..\\")
settings.Resolution = (1280, 720)

camera = CXXCamera()
camera.Position = (6, -3, 5)
rotQuat = mathutils.Euler((0.9, 0.0, 1.1)).to_quaternion()
camera.Rotation = (rotQuat.w, rotQuat.x, rotQuat.y, rotQuat.z)
camera.Result = "test_render.png"

light = CXXLight()
light.Intensity = 100
light.Position = (3, -4.2, 5)

mesh = CXXMesh()
mesh.Name = "cube"

scene = SceneProxy(settings, camera, [light], [mesh])

RenderScenes([scene])
