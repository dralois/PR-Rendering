from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from .Base import ObjectWrapper, DataWrapper

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

# Camera descriptor
class CameraData(DataWrapper):

    def __init__(self, blueprintID, cpy : DataWrapper = None):
        self.__camera : bpy.types.Camera

        if cpy is None:
            self.__camera = bpy.data.cameras.new(blueprintID)
        elif isinstance(cpy, CameraData):
            self.__camera = cpy.Blueprint.copy()
            self.__camera.name = blueprintID
        else:
            raise TypeError

        super().__init__(self.__camera)

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

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        self.CameraFOV = data.get("fov", (0.6911,0.4711))
        self.CameraShift = data.get("shift", (0.0,0.0))

# Camera object in scene
class CameraInstance(ObjectWrapper):

    def __init__(self, blueprint : CameraData):
        assert isinstance(blueprint, CameraData)
        super().__init__(blueprint)
        # Store descriptor
        self.__camData : CameraData
        self.__camData = blueprint
        self.__result = ""

    # Get camera data
    @property
    def Blueprint(self) -> CameraData:
        return self.__camData

    # Get result file name
    @property
    def CameraResultFile(self):
        return self.__result

    # Set result file name
    @CameraResultFile.setter
    def CameraResultFile(self, value):
        self.__result = value

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        super().CreateFromJSON(data)
        self.CameraResultFile = data.get("result", "")
