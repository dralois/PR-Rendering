from ..Utils.Importer import ImportBpy
from .Base import ObjectWrapper, DataWrapper

# Blender for multiprocessing
bpy = ImportBpy()

# Camera descriptor
class CameraData(DataWrapper):

    def __init__(self, name, cpy = None):
        self.__camera : bpy.types.Camera

        if cpy is None:
            self.__camera = bpy.data.cameras.new("cam_" + name)
        elif isinstance(cpy, CameraData):
            self.__camera = cpy.Blueprint.copy()
            self.__camera.name = name
        else:
            raise TypeError

        super().__init__(name, self.__camera)

    # Override: Cleanup & remove camera
    def _Cleanup(self):
        if self.__camera is not None:
            bpy.data.cameras.remove(self.__camera)

    # Override: Get if camera valid
    @property
    def Valid(self):
        return self.__camera is not None

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

# Camera object in scene
class CameraInstance(ObjectWrapper):

    def __init__(self, name, result, data : CameraData):
        super().__init__(name, data)
        # Store descriptor
        self.__camData : CameraData
        self.__camData = data
        self.__result = result

    # Get camera data
    @property
    def Blueprint(self) -> CameraData:
        return self.__camData

    # Get result file name
    @property
    def CameraResultFile(self):
        return self.__result
