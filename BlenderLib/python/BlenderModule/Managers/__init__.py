__all__ = ["SceneManager", "MaterialManager"]

import bpy

from . import SceneManager
from . import MaterialManager

# Initializes blender for rendering
def Initialize(pluginPath, outputPath):
    # Init blenderseed plugin
    try:
        bpy.ops.preferences.addon_enable(module = "blenderseed")
    except:
        bpy.ops.preferences.addon_install(overwrite = False, filepath = pluginPath + "\\blenderseed.zip")
        bpy.ops.preferences.addon_enable(module = "blenderseed")
    # Remove default stuff from scene
    bpy.data.batch_remove([obj.data for obj in bpy.data.objects])
    bpy.data.batch_remove([mat for mat in bpy.data.materials])
    # Change renderer to appleseed
    bpy.context.scene.render.engine = "APPLESEED_RENDER"
    # Set render settings
    bpy.context.scene.render.image_settings.file_format = "PNG"
    bpy.context.scene.render.resolution_x = 1920
    bpy.context.scene.render.resolution_y = 1080
    bpy.context.scene.render.filepath = outputPath + "\\output.png"
