from typing import List

import bpy
import mathutils

from ..Utils.BridgeObjects import CXXMesh, CXXLight, CXXCamera, CXXSettings
from ..Converters import Camera, Mesh, Lights
from . import ObjectManager

# TODO
class Scene(object):

    def __init__(self, ):
        self.__isActive = False
        self.__settings = CXXSettings()
        self.__camera : Camera.CameraConverter = None
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

    # Initialize scene for rendering
    def Setup(self):
        # Init blenderseed plugin
        try:
            bpy.ops.preferences.addon_refresh()
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        except ImportError:
            bpy.ops.preferences.addon_install(filepath = self.__settings.Plugin)
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        # Remove default stuff from scene
        bpy.data.batch_remove([obj.data for obj in bpy.data.objects])
        bpy.data.batch_remove([mat for mat in bpy.data.materials])
        # Change renderer to appleseed
        bpy.context.scene.render.engine = "APPLESEED_RENDER"

    # Render scene (camera)
    def Render(self):
        self.__isActive = True
        # Activate scenes' camera for rendering
        bpy.context.scene.camera = self.__camera.BlenderObject()
        # Adjust render settings
        bpy.context.scene.render.filepath = self.__settings.Output + self.__camera.CameraResultFile
        bpy.context.scene.render.image_settings.compression = (15, 0)[self.__settings.DepthOnly]
        bpy.context.scene.render.image_settings.color_depth = (8, 32)[self.__settings.DepthOnly]
        bpy.context.scene.render.image_settings.color_mode = ("RGBA", "BW")[self.__settings.DepthOnly]
        bpy.context.scene.render.image_settings.file_format = "PNG"
        bpy.context.scene.render.image_settings.use_zbuffer = not self.__settings.DepthOnly
        bpy.context.scene.render.resolution_x = self.__settings.Resolution[0]
        bpy.context.scene.render.resolution_y = self.__settings.Resolution[1]
        # Render scene to file
        bpy.ops.render.render(write_still = True)
        self.__isActive = False

    # Get output settings
    @property
    def Settings(self):
        return self.__settings

    # Set output settings
    @Settings.setter
    def Settings(self, value):
        self.__settings = value

    # Get scene camera
    @property
    def Camera(self):
        return self.__camera

    # Set scene camera
    @Camera.setter
    def Camera(self, value):
        self.__camera = value

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

class SceneProxy(object):
    def __init__(self, settings, camera, lights, meshes):
        self.Settings : CXXSettings = settings
        self.Camera : CXXCamera = camera
        self.Lights : List[CXXLight] = lights
        self.Meshes : List[CXXMesh] = meshes

# TODO
# Build and return scene from proxy
def Proxy2Scene(proxy : SceneProxy):
    # Create scene & settings
    build = Scene()
    build.Settings = proxy.Settings

    # Build camera
    sceneCam = Camera.CameraConverter("scene", proxy.Camera.result)
    sceneCam.CameraFOV = proxy.Camera.FOV[0], proxy.Camera.FOV[1]
    sceneCam.CameraShift = proxy.Camera.Shift[0], proxy.Camera.Shift[1]
    sceneCam.ObjectPosition = mathutils.Vector(proxy.Camera.Position)
    sceneCam.ObjectRotationQuat = mathutils.Quaternion(proxy.Camera.Rotation)
    sceneCam.ObjectScale = mathutils.Vector(proxy.Camera.Scale)
    build.Camera = sceneCam

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
