from typing import List

import bpy
import mathutils

from ..BridgeObjects import CXXMesh, CXXLight, CXXCamera
from ..Converters import Camera, Mesh, Lights
from . import ObjectManager

class Scene(object):

    def __init__(self):
        self.__isActive = False
        self.__output = ""
        self.__camera = Camera.CameraConverter()
        self.__meshes = ObjectManager.MeshFactory()
        self.__lights = ObjectManager.LightFactory()
        self.__materials = ObjectManager.MaterialFactory()

    def __del__(self):
        # Make sure rendering is cancelled
        if(self.__isActive):
            bpy.ops.wm.quit_blender()
        del self.__camera
        del self.__meshes
        del self.__lights
        del self.__materials

    # Render scene (camera)
    def Render(self):
        self.__isActive = True
        # Set render camera & output
        bpy.context.scene.render.filepath += self.__output
        bpy.context.scene.camera = self.__camera.BlenderObject()
        # Render scene to file
        bpy.ops.render.render(write_still = True)
        self.__isActive = False

    # Get output file
    @property
    def Output(self):
        return self.__output

    # Set output file
    @Output.setter
    def Output(self, value):
        self.__output = value

    # Get scene camera
    @property
    def Camera(self):
        return self.__camera

    # Get light manager
    @property
    def LightManager(self):
        return self.__lights

    # Get mesh manager
    @property
    def MeshManager(self):
        return self.__meshes

    # Get material manager
    @property
    def MaterialManager(self):
        return self.__materials

# TODO
class SceneProxy(object):
    def __init__(self, meshes, lights, camera):
        self.Camera : CXXCamera = camera
        self.Lights : List[CXXLight] = lights
        self.Meshes : List[CXXMesh] = meshes

# TODO
# Build and return scene from proxy
def Proxy2Scene(proxy : SceneProxy):
    # Create scene
    build = Scene()
    build.Output = proxy.Camera.Output

    # Build camera
    build.Camera.CameraFOV = proxy.Camera.FOV[0], proxy.Camera.FOV[1]
    build.Camera.CameraShift = proxy.Camera.Shift[0], proxy.Camera.Shift[1]
    build.Camera.ObjectPosition = mathutils.Vector(proxy.Camera.Position)
    build.Camera.ObjectRotationQuat = mathutils.Quaternion(proxy.Camera.Rotation)
    build.Camera.ObjectScale = mathutils.Vector(proxy.Camera.Scale)

    # TODO
    # Build meshes
    mesh : CXXMesh
    for mesh in proxy.Meshes:
        # FIXME: mesh.Shader -> material
        curr = build.MeshManager.CreateMesh(mesh.Name, mesh.File, None)
        curr.ObjectPosition = mathutils.Vector(mesh.Position)
        curr.ObjectRotationQuat = mathutils.Quaternion(mesh.Rotation)
        curr.ObjectScale = mathutils.Vector(mesh.Scale)

    # TODO
    # Build lights
    light : CXXLight
    for light in proxy.Lights:
        # FIXME: Name, types, params
        curr = build.LightManager.CreateLight("default", "POINT")
        curr.LightIntensity = light.Intensity
        curr.ObjectPosition = mathutils.Vector(light.Position)
        curr.ObjectRotationQuat = mathutils.Quaternion(light.Rotation)
        curr.ObjectScale = mathutils.Vector(light.Scale)

    # Return scene
    return build
