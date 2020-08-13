from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from ..Utils import FullPath
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

    # Get FOV [aspect, x, y]
    @property
    def CameraFOV(self):
        aspect = self.__camera.sensor_width / self.__camera.sensor_height
        return aspect, self.__camera.angle_x, self.__camera.angle_y

    # Set FOV [aspect, x, y]
    @CameraFOV.setter
    def CameraFOV(self, value):
        self.__camera.sensor_height = self.__camera.sensor_width / value[0]
        self.__camera.angle_x = value[1]
        self.__camera.angle_y = value[2]

    # Get shift [x, y]
    @property
    def CameraShift(self):
        return self.__camera.shift_x, self.__camera.shift_y

    # Set shift [x, y]
    @CameraShift.setter
    def CameraShift(self, value):
        self.__camera.shift_x = value[0]
        self.__camera.shift_y = value[1]

    # Get near z plane distance
    @property
    def CameraNearZ(self):
        return self.__camera.appleseed.near_z

    # Set near z plane distance
    @CameraNearZ.setter
    def CameraNearZ(self, value):
        nearZ = (-1.0 * value, value)[value < 0.0]
        self.__camera.appleseed.near_z = nearZ

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        self.CameraFOV = data.get("fov", (1.5, 0.6911, 0.4711))
        self.CameraShift = data.get("shift", (0.0, 0.0))
        self.CameraNearZ = data.get("nearZ", 0.001)

# Camera object in scene
class CameraInstance(ObjectWrapper):

    def __init__(self, blueprint : CameraData):
        assert isinstance(blueprint, CameraData)
        super().__init__(blueprint)
        # Store descriptor
        self.__camData : CameraData
        self.__camData = blueprint
        # Render settings
        self.__result = ""
        self.__resolution = (1920, 1080)
        self.__dataOnly = False
        self.__aaSamples = 16
        self.__rayBounces = -1
        self.__shadingOverride = ""

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

    # Get render resolution
    @property
    def CameraResolution(self):
        return self.__resolution

    # Set render resolution
    @CameraResolution.setter
    def CameraResolution(self, value):
        self.__resolution = value

    # Get data / color rendering
    @property
    def CameraDataOnly(self):
        return self.__dataOnly

    # Set data / color rendering
    @CameraDataOnly.setter
    def CameraDataOnly(self, value):
        self.__dataOnly = value

    # Get anti aliasing sample count
    @property
    def CameraAASamples(self):
        return self.__aaSamples

    # Set anti aliasing sample count
    @CameraAASamples.setter
    def CameraAASamples(self, value):
        self.__aaSamples = value

    # Get max total bounces
    @property
    def CameraRayBounces(self):
        return self.__rayBounces

    # Set max total bounces
    @CameraRayBounces.setter
    def CameraRayBounces(self, value):
        self.__rayBounces = value

    # Get shading override to use
    @property
    def CameraShadingOverride(self):
        return self.__shadingOverride

    # Set shading override to use
    @CameraShadingOverride.setter
    def CameraShadingOverride(self, value):
        self.__shadingOverride = value

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        super().CreateFromJSON(data)
        self.CameraResultFile = data.get("resultFile", "")
        self.CameraResolution = data.get("resolution", (1920, 1080))
        self.CameraDataOnly = data.get("dataOnly", False)
        self.CameraAASamples = data.get("aaSamples", 16)
        self.CameraRayBounces = data.get("rayBounces", -1)
        self.CameraShadingOverride = data.get("shadingOverride", "")
