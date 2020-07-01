from .Managers.RenderManager import RenderScenes, RenderSceneSingle
from .Managers.SceneManager import SceneProxy
from .Utils.BridgeObjects import CXXCamera, CXXLight, CXXMesh, CXXSettings

import os
import mathutils

settings = CXXSettings()
settings.Output = os.path.join(os.getcwd(), "BlenderModule\\Test\\")
settings.Plugin = os.path.join(os.getcwd(), "blenderseed.zip")
settings.Shaders = os.path.join(os.getcwd(), "BlenderModule\\Test\\")
settings.Resolution = (1280, 720)
settings.LogLevel = "info"

camera = CXXCamera()
camera.Position = (6.0, -3.0, 5.0)
rotQuat = mathutils.Euler((0.9, 0.0, 1.1)).to_quaternion()
camera.Rotation = (rotQuat.w, rotQuat.x, rotQuat.y, rotQuat.z)
camera.Result = "test_render.png"

light = CXXLight()
light.Name = "Default"
light.Intensity = 100
light.Position = (3.0, -4.2, 5.0)

mesh1 = CXXMesh()
mesh1.Name = "cube1"
mesh1.File = os.path.join(os.getcwd(), "BlenderModule\\Test\\module_test.obj")
mesh1.Shader = "asModuleTest"
mesh1.Position = (-0.5, -0.5, 0.0)
mesh1.Scale = (0.5, 0.5, 0.5)

mesh2 = CXXMesh()
mesh2.Name = "cube2"
mesh2.File = os.path.join(os.getcwd(), "BlenderModule\\Test\\module_test.obj")
mesh2.Shader = "asModuleTest"
mesh2.Position = (1.0, 1.0, 0.0)
mesh2.Scale = (0.5, 0.5, 0.5)

scene = SceneProxy(settings, camera, [light], [mesh1, mesh2])

RenderSceneSingle(scene)
