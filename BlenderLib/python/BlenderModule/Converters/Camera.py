import bpy
import mathutils

from .Base import ObjectConverter

class CameraConverter(ObjectConverter):

    def __init__(self, name):
        self.__camera : bpy.types.Camera = bpy.data.cameras.new("cam_" + name)
        super().__init__(name, self.__camera)

    def __del__(self):
        super().__del__()

    # Get FOV [x, y]
    @property
    def CameraFOV(self):
        return self.__camera.angle_x, self.__camera.angle_y

    # Set FOV [x, y]
    @CameraFOV.setter
    def CameraFOV(self, value):
        self.__camera.angle_x = value[0]
        self.__camera.angle_y = value[1]

    # Get shift [x, y]
    @property
    def CameraShift(self):
        return self.__camera.shift_x, self.__camera.shift_y

    # Set shift [x, y]
    @CameraShift.setter
    def CameraShift(self, value):
        self.__camera.shift_x = value[0]
        self.__camera.shift_y = value[1]
