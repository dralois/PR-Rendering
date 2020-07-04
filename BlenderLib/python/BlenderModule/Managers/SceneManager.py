from ..Utils.Importer import ImportBpy
from ..Utils.ShaderCompiler import CompileFolder
from ..Utils.Logger import GetLogger, GetLevel, SetLevel
from ..Converters import Camera, Lights, Material, Mesh, Shader
from . import ObjectManager

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
        bpy.context.scene.camera = self.__camera.ObjectInstance
        # Adjust render settings
        bpy.context.scene.render.filepath = self.__settings.get("output", "output\\") + self.__camera.CameraResultFile
        bpy.context.scene.render.image_settings.compression = (15, 0)[self.__settings.get("depthOnly", False)]
        bpy.context.scene.render.image_settings.color_depth = ("8", "32")[self.__settings.get("depthOnly", False)]
        bpy.context.scene.render.image_settings.color_mode = ("RGBA", "BW")[self.__settings.get("depthOnly", False)]
        bpy.context.scene.render.image_settings.file_format = "PNG"
        bpy.context.scene.render.image_settings.use_zbuffer = not self.__settings.get("depthOnly", False)
        bpy.context.scene.render.resolution_x = self.__settings.get("resolution", (1920, 1080))[0]
        bpy.context.scene.render.resolution_y = self.__settings.get("resolution", (1920, 1080))[1]
        # Render scene to file
        bpy.ops.render.render(write_still = True)
        self.__isActive = False

    # TODO
    # Initialize scene for rendering
    def __Setup(self):
        logger.info("Setting up scene")
        # Compile all shaders
        asPath = bpy.utils.user_resource("SCRIPTS", "addons") + "\\blenderseed"
        searchPaths = os.path.abspath(os.path.dirname(__file__) +  "\\..\\Test\\")
        # For each shader path
        for shaderPath in self.__settings.get("shaderPaths", ["shaders\\"]):
            fullPath = os.path.abspath(shaderPath)
            # Compile and add to appleseed search
            CompileFolder(fullPath, asPath)
            searchPaths += os.path.pathsep + fullPath
        # Set shader searchpath
        os.environ["APPLESEED_SEARCHPATH"] = searchPaths
        # TODO: Read & convert textures from paths

        # Init blenderseed plugin
        try:
            bpy.ops.preferences.addon_refresh()
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        except:
            pluginPath = self.__settings.get("plugin", "blenderseed.zip")
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
