from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from .Base import DataWrapper, ObjectWrapper

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

import mathutils

# Generic light descriptor
class GenericLightData(DataWrapper):

    def __init__(self, impl : bpy.types.Light):
        assert isinstance(impl, bpy.types.Light)
        # Area light differs from rest
        self.__isArea = isinstance(impl, bpy.types.AreaLight)
        self.__impl : bpy.types.Light
        self.__impl = impl
        super().__init__(self.__impl)

    # Override: Cleanup & remove light
    def _Cleanup(self):
        if self.__impl is not None:
            bpy.data.lights.remove(self.__impl)

    # Override: Get if light valid
    @property
    def Valid(self):
        return self.__impl is not None

    # Get light intensity
    @property
    def LightIntensity(self):
        if self.__isArea:
            return self.__impl.appleseed.area_intensity
        else:
            return self.__impl.appleseed.radiance_multiplier

    # Set light intensity
    @LightIntensity.setter
    def LightIntensity(self, value):
        if self.__isArea:
            self.__impl.appleseed.area_intensity = value
        else:
            self.__impl.appleseed.radiance_multiplier = value

    # Get exposure
    @property
    def LightExposure(self):
        if self.__isArea:
            return self.__impl.appleseed.area_exposure
        else:
            return self.__impl.appleseed.exposure

    # Set exposure
    @LightExposure.setter
    def LightExposure(self, value):
        if self.__isArea:
            self.__impl.appleseed.area_exposure = value
        else:
            self.__impl.appleseed.exposure = value

    # Get whether light casts indirect light
    @property
    def LightCastsIndirect(self):
        return self.__impl.appleseed.cast_indirect

    # Set whether light casts indirect light
    @LightCastsIndirect.setter
    def LightCastsIndirect(self, value):
        self.__impl.appleseed.cast_indirect = value

    # Get light color
    @property
    def LightColor(self) -> mathutils.Color:
        if self.__isArea:
            return self.__impl.appleseed.area_color
        else:
            return self.__impl.appleseed.radiance

    # Set light color
    @LightColor.setter
    def LightColor(self, value):
        if self.__isArea:
            self.__impl.appleseed.area_color = value
        else:
            self.__impl.appleseed.radiance = value

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        self.LightColor = mathutils.Vector(data.get("color", (1.0,1.0,1.0)))
        self.LightIntensity = data.get("intensity", 1.0)
        self.LightExposure = data.get("exposure", 1.0)
        self.LightCastsIndirect = data.get("castsIndirect", True)

# Point light descriptor
class PointLightData(GenericLightData):

    def __init__(self, blueprintID, cpy = None):
        self.__light : bpy.types.PointLight

        if cpy is None:
            self.__light = bpy.data.lights.new(blueprintID, type="POINT")
        elif isinstance(cpy, PointLightData):
            self.__light = cpy.Blueprint.copy()
            self.__light.name = blueprintID
        else:
            raise TypeError

        super().__init__(self.__light)

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        super().CreateFromJSON(data)

# Spot light descriptor
class SpotLightData(GenericLightData):

    def __init__(self, blueprintID, cpy = None):
        self.__light : bpy.types.SpotLight

        if cpy is None:
            self.__light = bpy.data.lights.new(blueprintID, type="SPOT")
        elif isinstance(cpy, SpotLightData):
            self.__light = cpy.Blueprint.copy()
            self.__light.name = blueprintID
        else:
            raise TypeError

        super().__init__(self.__light)

    # Get spot angle in rad
    @property
    def SpotAngle(self):
        return self.__light.spot_size

    # Set spot angle in rad
    @SpotAngle.setter
    def SpotAngle(self, value):
        self.__light.spot_size = value

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        super().CreateFromJSON(data)
        # Try to parse param data
        if "params" in data:
            params = data.get("params")
            self.SpotAngle = params.get("spotAngle", 3.14)

# Sun light descriptor
class SunLightData(GenericLightData):

    def __init__(self, blueprintID, cpy = None):
        self.__light : bpy.types.SunLight

        if cpy is None:
            self.__light = bpy.data.lights.new(blueprintID, type="SUN")
        elif isinstance(cpy, SunLightData):
            self.__light = cpy.Blueprint.copy()
            self.__light.name = blueprintID
        else:
            raise TypeError

        # Directional light is not supported
        self.__light.appleseed.sun_mode = "sun"
        super().__init__(self.__light)

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        super().CreateFromJSON(data)

# Area light descriptor
class AreaLightData(GenericLightData):

    def __init__(self, blueprintID, cpy = None):
        self.__light : bpy.types.AreaLight

        if cpy is None:
            self.__light = bpy.data.lights.new(blueprintID, type="AREA")
        elif isinstance(cpy, bpy.type.AreaLight):
            self.__light = cpy.Blueprint.copy()
            self.__light.name = blueprintID
        else:
            raise TypeError

        super().__init__(self.__light)

    # Get area shape (RECTANGLE, DISK, SQUARE)
    @property
    def AreaShape(self):
        return self.__light.shape

    # Set area shape (RECTANGLE, DISK, SQUARE)
    @AreaShape.setter
    def AreaShape(self, value):
        self.__light.shape = value

    # Get area size [x, y]
    @property
    def AreaSize(self):
        return self.__light.size, self.__light.size_y

    # Set area size [x, y]
    @AreaSize.setter
    def AreaSize(self, value):
        self.__light.size = value[0]
        self.__light.size = value[1]

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        super().CreateFromJSON(data)
        # Try to parse param data
        if "params" in data:
            params = data.get("params")
            self.AreaShape = params.get("shape", "DISK")
            self.AreaSize = params.get("size", (1.0,1.0))

# Generic light object in scene
class GenericLightInstance(ObjectWrapper):

    def __init__(self, blueprint : GenericLightData):
        assert isinstance(blueprint, GenericLightData)
        super().__init__(blueprint)

    # Forwarded: Get light data
    @property
    def Blueprint(self) -> GenericLightData:
        raise NotImplementedError

# Point light object in scene
class PointLightInstance(GenericLightInstance):

    def __init__(self, blueprint : PointLightData):
        assert isinstance(blueprint, PointLightData)
        super().__init__(blueprint)
        # Store descriptor
        self.__lightData : PointLightData
        self.__lightData = blueprint

    # Override: Get point light data
    @property
    def Blueprint(self) -> PointLightData:
        return self.__lightData

# Spot light object in scene
class SpotLightInstance(GenericLightInstance):

    def __init__(self, blueprint : SpotLightData):
        assert isinstance(blueprint, SpotLightData)
        super().__init__(blueprint)
        # Store descriptor
        self.__lightData : SpotLightData
        self.__lightData = blueprint

    # Override: Get spot light data
    @property
    def Blueprint(self) -> SpotLightData:
        return self.__lightData

# Sun light object in scene
class SunLightInstance(GenericLightInstance):

    def __init__(self, blueprint : SunLightData):
        assert isinstance(blueprint, SunLightData)
        super().__init__(blueprint)
        # Store descriptor
        self.__lightData : SunLightData
        self.__lightData = blueprint

    # Override: Get sun light data
    @property
    def Blueprint(self) -> SunLightData:
        return self.__lightData

# Area light object in scene
class AreaLightInstance(GenericLightInstance):

    def __init__(self, blueprint : AreaLightData):
        assert isinstance(blueprint, AreaLightData)
        super().__init__(blueprint)
        # Store descriptor
        self.__lightData : AreaLightData
        self.__lightData = blueprint

    # Override: Get area light data
    @property
    def Blueprint(self) -> AreaLightData:
        return self.__lightData
