
from .Managers import RenderManager
from .Managers.SceneManager import SceneProxy
from .Utils.BridgeObjects import CXXCamera, CXXLight, CXXMesh, CXXSettings

import mathutils

settings = CXXSettings()
settings.Output = "test//"
settings.Plugin = "..//"
settings.Resolution = (1280, 720)

camera = CXXCamera()
camera.Position = (6, -3, 5)
rotQuat = mathutils.Euler((0.9, 0.0, 1.1)).to_quaternion()
camera.Rotation = (rotQuat.w, rotQuat.x, rotQuat.y, rotQuat.z)
camera.Result = "output.png"

light = CXXLight()
light.Intensity = 100
light.Position = (3, -4.2, 5)

mesh = CXXMesh()
mesh.Name = "cube"

scene = SceneProxy(settings, camera, [light], [mesh])

RenderManager.RenderScenes([scene])
