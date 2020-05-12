import bpy
import mathutils

from .Base import Converter

class CameraConverter(Converter):

    def __init__(self, name):
        super().__init__(name)
        self.__isActive = False
        self.__fullname = "cam_persp_" + self._name
        self.__camera : bpy.types.Camera = bpy.data.cameras.new(self.__fullname)

    def __del__(self):
        super().__del__()

    # Creates new camera
    def CreateCamera(self, fovX, fovY, shiftX, shiftY):
        # Set camera parameters
        self.__camera.angle_x = fovX
        self.__camera.angle_y = fovY
        self.__camera.shift_x = shiftX
        self.__camera.shift_y = shiftY
        # Store in scene
        self._CreateObject(self.__camera)

    # Renders this camera
    def ActivateAndRender(self):
        self.__isActive = True
        # Set active camera to this one
        bpy.context.scene.camera = self.BlenderObject
        # Render scene to file
        bpy.ops.render.render(write_still = True)
        self.__isActive = False
