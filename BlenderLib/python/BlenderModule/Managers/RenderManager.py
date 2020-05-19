from multiprocessing import Process
from multiprocessing.managers import BaseManager

import bpy

from . import SceneManager

# TODO
class RenderProcess(Process):

    def __init__(self, paths, resolution, scene):
        super().__init__()
        self.__paths = paths
        self.__resolution = resolution
        self.__scene : SceneManager.SceneProxy = scene

    def __del__(self):
        super().close()

    # TODO
    # Called when invoking start()
    def run(self):
        # Apply render settings
        self.__SetupRender()
        # Create scene from proxy
        scene = SceneManager.Proxy2Scene(self.__scene)
        # Finally render the scene
        scene.Render()

    # Setup appleseed & output
    def __SetupRender(self):
        # Init blenderseed plugin
        try:
            bpy.ops.preferences.addon_refresh()
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        except:
            bpy.ops.preferences.addon_install(filepath = self.__paths[1])
            bpy.ops.preferences.addon_enable(module = "blenderseed")
        # Remove default stuff from scene
        bpy.data.batch_remove([obj.data for obj in bpy.data.objects])
        bpy.data.batch_remove([mat for mat in bpy.data.materials])
        # Change renderer to appleseed
        bpy.context.scene.render.engine = "APPLESEED_RENDER"
        # Adjust render settings
        bpy.context.scene.render.image_settings.file_format = "PNG"
        bpy.context.scene.render.image_settings.color_mode = "RGBA"
        bpy.context.scene.render.resolution_x = self.__resolution[0]
        bpy.context.scene.render.resolution_y = self.__resolution[1]
        bpy.context.scene.render.filepath = self.__paths[0]

class RenderProxy(BaseManager):
    pass

RenderProxy.register("SceneProxy", SceneManager.SceneProxy)
