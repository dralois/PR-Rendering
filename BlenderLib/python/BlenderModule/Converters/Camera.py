from ..Utils.Importer import DoImport

# Blender for multiprocessing
bpy = DoImport()

from .Base import ObjectConverter

class CameraConverter(ObjectConverter):

    def __init__(self, name, result):
        self.__result = result
        self.__camera : bpy.types.Camera = bpy.data.cameras.new("cam_" + name)
        super().__init__(name, self.__camera)

    def __del__(self):
        super().__del__()

    # Get result file name
    @property
    def CameraResultFile(self):
        return self.__result

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
