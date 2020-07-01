from ..Utils.Importer import ImportBpy
from ..Utils.BridgeObjects import CXXMesh, CXXLight, CXXCamera, CXXSettings
from ..Utils.ShaderCompiler import CompileFolder
from ..Utils.Logger import get_logger, get_logger_level
from ..Converters import Camera, Lights, Material, Mesh, Shader
from . import ObjectManager

# Blender for multiprocessing
bpy = ImportBpy()

from typing import List
import mathutils
import os

logger = get_logger()

# Actual blender data storage & rendering class
class Scene(object):

    def __init__(self, ):
        self.__isActive = False
        self.__settings = CXXSettings()
        self.__camera : Camera.CameraInstance
        self.__camera = None
        self.__meshes = ObjectManager.MeshFactory()
        self.__lights = ObjectManager.LightFactory()

    def __del__(self):
        # Make sure rendering is cancelled
        if(self.__isActive):
            bpy.ops.wm.quit_blender()
        del self.__camera
        del self.__meshes
        del self.__lights

    # Render scene (camera)
    def Render(self):
        logger.info(f"Rendering scene to {self.Camera.CameraResultFile}")
        # Activate scenes' camera for rendering
        self.__isActive = True
        bpy.context.scene.camera = self.__camera.ObjectInstance
        # Adjust render settings
        bpy.context.scene.render.filepath = self.__settings.Output + self.__camera.CameraResultFile
        bpy.context.scene.render.image_settings.compression = (15, 0)[self.__settings.DepthOnly]
        bpy.context.scene.render.image_settings.color_depth = ("8", "32")[self.__settings.DepthOnly]
        bpy.context.scene.render.image_settings.color_mode = ("RGBA", "BW")[self.__settings.DepthOnly]
        bpy.context.scene.render.image_settings.file_format = "PNG"
        bpy.context.scene.render.image_settings.use_zbuffer = not self.__settings.DepthOnly
        bpy.context.scene.render.resolution_x = self.__settings.Resolution[0]
        bpy.context.scene.render.resolution_y = self.__settings.Resolution[1]
        # Render scene to file
        bpy.ops.render.render(write_still = True)
        self.__isActive = False

    # Initialize scene for rendering
    def __Setup(self):
        logger.info("Setting up scene")
        # Compile all shaders
        asPath = bpy.utils.user_resource('SCRIPTS', "addons") + "//blenderseed"
        CompileFolder(self.__settings.Shaders, asPath)
        # Set shader searchpath
        os.environ["APPLESEED_SEARCHPATH"] = "".join([self.__settings.Shaders, 
            os.path.pathsep, os.path.join(os.path.dirname(__file__), "..\\Test\\")])
        # Init blenderseed plugin
        try:
            bpy.ops.preferences.addon_refresh()
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        except:
            bpy.ops.preferences.addon_install(filepath = self.__settings.Plugin)
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        # Remove default stuff from scene
        bpy.data.batch_remove([obj.data for obj in bpy.data.objects])
        bpy.data.batch_remove([mat for mat in bpy.data.materials])
        # Change renderer to appleseed
        bpy.context.scene.render.engine = "APPLESEED_RENDER"
        # Change appleseed debug output level to match own logger
        bpy.context.preferences.addons["blenderseed"].preferences.log_level = get_logger_level()

    # Get output settings
    @property
    def Settings(self):
        return self.__settings

    # Set output settings & setup blenderseed
    @Settings.setter
    def Settings(self, value):
        self.__settings = value
        self.__Setup()

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

# Proxy data storage class
class SceneProxy(object):
    def __init__(self, settings, camera, lights, meshes):
        self.Settings : CXXSettings = settings
        self.Camera : CXXCamera = camera
        self.Lights : List[CXXLight] = lights
        self.Meshes : List[CXXMesh] = meshes

# Build and return scene from proxy
def Proxy2Scene(proxy : SceneProxy):
    # Create scene & settings
    build : Scene = Scene()
    build.Settings = proxy.Settings

    # Build camera
    camBlueprint = Camera.CameraData("default")
    camBlueprint.CameraFOV = proxy.Camera.FOV[0], proxy.Camera.FOV[1]
    camBlueprint.CameraShift = proxy.Camera.Shift[0], proxy.Camera.Shift[1]
    build.Camera = Camera.CameraInstance("scene", proxy.Camera.Result, camBlueprint)
    build.Camera.ObjectPosition = mathutils.Vector(proxy.Camera.Position)
    build.Camera.ObjectRotationQuat = mathutils.Quaternion(proxy.Camera.Rotation)
    build.Camera.ObjectScale = mathutils.Vector(proxy.Camera.Scale)

    # Build meshes
    mesh : CXXMesh
    for mesh in proxy.Meshes:
        meshBlueprint = build.MeshManager.GetMeshBlueprint(mesh.File)
        meshShader = Shader.GetShader(mesh.Shader)(mesh.Shader)
        meshInstance = build.MeshManager.AddMeshInstance(mesh.Name, meshBlueprint, meshShader)
        meshInstance.ObjectPosition = mathutils.Vector(mesh.Position)
        meshInstance.ObjectRotationQuat = mathutils.Quaternion(mesh.Rotation)
        meshInstance.ObjectScale = mathutils.Vector(mesh.Scale)

    # Build lights
    light : CXXLight
    for light in proxy.Lights:
        # FIXME: Params
        lightBlueprint = build.LightManager.GetLightBlueprint(light.Type)
        lightBlueprint.LightColor = light.Color
        lightBlueprint.LightIntensity = light.Intensity
        lightBlueprint.LightExposure = light.Exposure
        lightBlueprint.LightCastsIndirect = light.CastsIndirect
        lightInstance = build.LightManager.AddLightInstance(light.Name, lightBlueprint)
        lightInstance.ObjectPosition = mathutils.Vector(light.Position)
        lightInstance.ObjectRotationQuat = mathutils.Quaternion(light.Rotation)
        lightInstance.ObjectScale = mathutils.Vector(light.Scale)

    # Return scene
    return build
