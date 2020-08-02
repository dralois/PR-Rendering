from ..Utils.Importer import ImportBpy
from ..Utils.ShaderCompiler import CompileFolder, EnsureInstalled
from ..Utils.Logger import GetLogger, GetLevel, SetLevel
from ..Utils import FileDir, FileName, FullFileName, FullPath
from ..Converters import Camera, Lights, Material, Mesh, Shader
from . import ObjectManager, TextureManager

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

from typing import List
import mathutils
import os

# Actual blender data storage & rendering class
class Scene(object):

    def __init__(self, ):
        self.__isActive = False
        self.__settings = {}
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
        ctx = bpy.context.scene
        ctx.camera = self.__camera.ObjectInstance
        ctx.view_settings.view_transform = ("Raw", "Standard")[self.__camera.CameraDataOnly]
        # Adjust render settings
        ctx.render.filepath = FullPath(self.__camera.CameraResultFile)
        ctx.render.image_settings.file_format = ("PNG", "OPEN_EXR")[self.__camera.CameraDataOnly]
        ctx.render.image_settings.compression = (15, 0)[self.__camera.CameraDataOnly]
        ctx.render.image_settings.color_mode = ("RGBA", "RGB")[self.__camera.CameraDataOnly]
        ctx.render.image_settings.color_depth = ("8", "32")[self.__camera.CameraDataOnly]
        ctx.render.resolution_x = self.__settings.get("resolution", (1920, 1080))[0]
        ctx.render.resolution_y = self.__settings.get("resolution", (1920, 1080))[1]
        ctx.render.use_compositing = False
        ctx.render.use_sequencer = False
        # Shading override
        if len(self.__camera.CameraShadingOverride) > 0:
            ctx.appleseed.shading_override = True
            ctx.appleseed.override_mode = self.__camera.CameraShadingOverride
        else:
            ctx.appleseed.shading_override = False
        # Adjust raytracing settings
        ctx.appleseed.use_embree = True
        ctx.appleseed.force_aa = (self.__camera.CameraAASamples > 1, False)[self.__camera.CameraDataOnly]
        ctx.appleseed.samples = (self.__camera.CameraAASamples, 1)[self.__camera.CameraDataOnly]
        ctx.appleseed.max_bounces_unlimited = self.__camera.CameraRayBounces < 0
        ctx.appleseed.max_bounces = self.__camera.CameraRayBounces
        ctx.appleseed.max_specular_bounces_unlimited = self.__camera.CameraRayBounces < 0
        ctx.appleseed.max_specular_bounces = self.__camera.CameraRayBounces
        ctx.appleseed.max_diffuse_bounces_unlimited = self.__camera.CameraRayBounces < 0
        ctx.appleseed.max_diffuse_bounces = self.__camera.CameraRayBounces
        # Possibly store to blend file
        if self.__settings.get("storeBlend", False):
            saveFile = f"{FileDir(self.__camera.CameraResultFile)}\\{FileName(self.__camera.CameraResultFile)}.blend"
            bpy.ops.wm.save_mainfile(filepath = saveFile, check_existing = False)
        # Render scene to file
        bpy.ops.render.render(write_still = True)
        self.__isActive = False

    # Initialize scene for rendering
    def __Setup(self):
        logger.info("Setting up scene")
        # Make sure blenderseed is available
        modulePath = f'{bpy.utils.user_resource("SCRIPTS", "addons")}\\blenderseed'
        pluginPath = FullPath(self.__settings.get("pluginPath", "..\\blenderseed.zip"))
        isInstalled = EnsureInstalled(modulePath, pluginPath)
        # Compile all shaders
        searchPaths = ""
        compilePaths = self.__settings.get("shaderDirs", [])
        compilePaths.append(FullPath(f"{FileDir(__file__)}\\..\\Shaders\\"))
        # Compile & add each shader directory
        for shaderPath in [FullPath(path) for path in compilePaths]:
            CompileFolder(shaderPath, modulePath)
            searchPaths += os.path.pathsep + shaderPath
        # Set shader searchpaths
        os.environ["APPLESEED_SEARCHPATH"] = searchPaths
        # Init texture system
        Shader.SetTextureSystem(TextureManager.TextureFactory(modulePath))
        # Init blenderseed plugin
        if isInstalled:
            bpy.ops.preferences.addon_refresh()
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        else:
            bpy.ops.preferences.addon_install(filepath = pluginPath)
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        # Remove default stuff from scene
        bpy.data.batch_remove([obj.data for obj in bpy.data.objects])
        bpy.data.batch_remove([mat for mat in bpy.data.materials])
        # Change renderer to appleseed
        bpy.context.scene.render.engine = "APPLESEED_RENDER"
        # Change appleseed debug output level to match own logger
        bpy.context.preferences.addons["blenderseed"].preferences.log_level = GetLevel()

    # Get output settings
    @property
    def Settings(self):
        return self.__settings

    # Set output settings & setup blenderseed
    @Settings.setter
    def Settings(self, value):
        self.__settings = value
        SetLevel(self.__settings.get("logLevel", "error"))
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

# Build and return scene from proxy
def CreateFromJSON(data : dict) -> Scene:
    # Create scene & settings
    build : Scene = Scene()
    build.Settings = data.get("settings", {})

    # Build camera
    camBlueprint = Camera.CameraData("scene")
    build.Camera = Camera.CameraInstance(camBlueprint)
    build.Camera.CreateFromJSON(data.get("camera", {}))

    # Build meshes
    for meshData in data.get("meshes", []):
        mesh = build.MeshManager.GetInstance(meshData)
        logger.info(f"Added {mesh.BlueprintID} to scene {build}")

    # Build lights
    for lightData in data.get("lights", []):
        light = build.LightManager.GetInstance(lightData)
        logger.info(f"Added {light.BlueprintID} to scene {build}")

    # Return scene
    return build
