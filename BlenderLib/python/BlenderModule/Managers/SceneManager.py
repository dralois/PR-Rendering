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
import sys
import os

# Actual blender data storage & rendering class
class Scene(object):

    def __init__(self, settings : dict):
        self.__storeBlend = False
        self.__cameras = ObjectManager.CameraFactory()
        self.__meshes = ObjectManager.MeshFactory()
        self.__lights = ObjectManager.LightFactory()
        self.__renderQueue : List[Camera.CameraInstance]
        self.__renderQueue = []
        # Setup blenderseed
        self.__SetGeneralSettings(settings)

    def __del__(self):
        del self.__cameras
        del self.__meshes
        del self.__lights
        del self.__renderQueue

    # Enque camera for rendering
    def RenderQueueAdd(self, camera : Camera.CameraInstance):
        assert isinstance(camera, Camera.CameraInstance)
        # Add camera at end of queue
        self.__renderQueue.append(camera)
        # Return enqued camera
        return camera

    # Remove all cameras from queue
    def RenderQueueClear(self):
        self.__renderQueue.clear()

    # Get remaining cameras in queue
    def RenderQueueRemaining(self):
        return len(self.__renderQueue)

    # Render scene (if camera queue not empty)
    def RenderQueueProcessNext(self):
        assert len(self.__renderQueue) > 0
        # Fetch camera from queue
        currCam = self.__renderQueue.pop(0)
        # Activate camera for rendering
        bpy.context.scene.camera = currCam.ObjectInstance
        # Adjust output & render settings
        self.__SetRenderSettings(currCam)
        # Adjust pathtracing & sampling settings
        self.__SetRaytracingSettings(currCam)
        # Store to blend file (last camera only)
        if self.__storeBlend and len(self.__renderQueue) == 0:
            saveFile = f"{FileDir(currCam.CameraResultFile)}\\Scene.blend"
            bpy.ops.wm.save_mainfile(filepath=saveFile, check_existing=False)
        # Render scene to file
        logger.info(f"Rendering scene to {currCam.CameraResultFile}")
        bpy.ops.render.render(write_still = True)

    # Sets up rendering from camera
    def __SetRenderSettings(self, camera : Camera.CameraInstance):
        ctx = bpy.context.scene
        # Color transformation
        ctx.render.use_sequencer = False
        ctx.render.use_compositing = False
        ctx.view_settings.view_transform = ("Standard", "Raw")[camera.CameraDataOnly]
        # Output format
        ctx.render.filepath = FullPath(camera.CameraResultFile)
        ctx.render.image_settings.file_format = ("PNG", "OPEN_EXR")[camera.CameraDataOnly]
        ctx.render.image_settings.compression = (15, 0)[camera.CameraDataOnly]
        ctx.render.image_settings.color_mode = ("RGBA", "RGB")[camera.CameraDataOnly]
        ctx.render.image_settings.color_depth = ("8", "32")[camera.CameraDataOnly]
        ctx.render.resolution_x = camera.CameraResolution[0]
        ctx.render.resolution_y = camera.CameraResolution[1]
        # Shading override
        ctx.appleseed.shading_override = len(camera.CameraShadingOverride) > 0
        if ctx.appleseed.shading_override:
            ctx.appleseed.override_mode = camera.CameraShadingOverride

    # Sets up raytracing from camera
    def __SetRaytracingSettings(self, camera : Camera.CameraInstance):
        ctx = bpy.context.scene
        # Sampling quality (If data: Rays through center of pixel)
        ctx.appleseed.use_embree = True
        ctx.appleseed.force_aa = (camera.CameraAASamples > 1, False)[camera.CameraDataOnly]
        ctx.appleseed.samples = (camera.CameraAASamples, 1)[camera.CameraDataOnly]
        # Pathtracing quality
        ctx.appleseed.max_bounces_unlimited = camera.CameraRayBounces < 0
        ctx.appleseed.max_bounces = camera.CameraRayBounces
        ctx.appleseed.max_specular_bounces_unlimited = camera.CameraRayBounces < 0
        ctx.appleseed.max_specular_bounces = camera.CameraRayBounces
        ctx.appleseed.max_diffuse_bounces_unlimited = camera.CameraRayBounces < 0
        ctx.appleseed.max_diffuse_bounces = camera.CameraRayBounces

    # Sets up general functionality
    def __SetGeneralSettings(self, settings : dict):
        # Logging & debug settings
        SetLevel(settings.get("logLevel", "error"))
        self.__storeBlend = settings.get("storeBlend", False)
        # Make sure blenderseed is available
        modulePath = f'{bpy.utils.user_resource("SCRIPTS", "addons")}\\blenderseed'
        pluginPath = FullPath(settings.get("pluginPath", "..\\blenderseed.zip"))
        isInstalled = EnsureInstalled(modulePath, pluginPath)
        # Compile all shaders
        searchPaths = ""
        compilePaths = settings.get("shaderDirs", [])
        compilePaths.append(FullPath(f"{FileDir(__file__)}\\..\\Shaders\\"))
        # Compile & add each shader directory (always in debug mode)
        forceCompile = logger.level < 30 and self.__storeBlend
        for shaderPath in [FullPath(path) for path in compilePaths]:
            CompileFolder(shaderPath, modulePath, forceCompile)
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

    # Get camera manager
    @property
    def CameraManager(self):
        return self.__cameras

    # Get light manager
    @property
    def LightManager(self):
        return self.__lights

    # Get mesh manager
    @property
    def MeshManager(self):
        return self.__meshes

# Build and return scene from json
def CreateFromJSON(data : dict) -> Scene:
    # Create scene & initialize blender
    build : Scene = Scene(data.get("settings", {}))

    # Build cameras & enque for rendering
    for camData in data.get("cameras", []):
        cam = build.RenderQueueAdd(build.CameraManager.GetInstance(camData))
        logger.info(f"{build}: Added {cam.Name}")

    # Build meshes
    for meshData in data.get("meshes", []):
        mesh = build.MeshManager.GetInstance(meshData)
        logger.info(f"{build}: Added {mesh.Name}")

    # Build lights
    for lightData in data.get("lights", []):
        light = build.LightManager.GetInstance(lightData)
        logger.info(f"{build}: Added {light.Name}")

    # Return scene
    return build

# Update and return scene from json
def UpdateFromJSON(data : dict, scene : Scene) -> Scene:
    # Remove & rebuild cameras
    scene.RenderQueueClear()
    scene.CameraManager.ClearInstances()
    for camData in data.get("cameras", []):
        cam = scene.RenderQueueAdd(scene.CameraManager.GetInstance(camData))
        logger.info(f"{scene}: Added {cam.Name}")

    # Update / create meshes
    for meshData in data.get("meshes", []):
        mesh = scene.MeshManager.UpdateInstance(meshData)
        if mesh is not None:
            logger.info(f"{scene}: Updated {mesh.Name}")
        else:
            mesh = scene.MeshManager.GetInstance(meshData)
            logger.info(f"{scene}: Added {mesh.Name}")

    # Remove meshes that weren't updated / created
    scene.MeshManager.ClearInstances()

    # Remove & rebuild lights
    scene.LightManager.ClearInstances()
    for lightData in data.get("lights", []):
        light = scene.LightManager.GetInstance(lightData)
        logger.info(f"{scene}: Added {light.Name}")

    # Return updated scene
    return scene
