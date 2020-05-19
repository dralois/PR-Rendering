import bpy
import mathutils

from .Base import ObjectConverter

class LightConverter(ObjectConverter):

    def __init__(self, name, impl):
        self.__isArea = isinstance(impl, bpy.types.AreaLight)
        self.__impl : bpy.types.Light = impl
        super().__init__(name, self.__impl)

    def __del__(self):
        super().__del__()

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

class PointLightConverter(LightConverter):

    def __init__(self, name):
        self.__light : bpy.types.PointLight = bpy.data.lights.new( "light_point_" + name, type="POINT")
        super().__init__(name, self.__light)

    def __del__(self):
        super().__del__()

class SpotLightConverter(LightConverter):

    def __init__(self, name):
        self.__light : bpy.types.SpotLight = bpy.data.lights.new("light_spot_" + name, type="SPOT")
        super().__init__(name, self.__light)

    def __del__(self):
        super().__del__()

    # Get spot angle in rad
    @property
    def SpotAngle(self):
        return self.__light.spot_size

    # Set spot angle in rad
    @SpotAngle.setter
    def SpotAngle(self, value):
        self.__light.spot_size = value

class SunLightConverter(LightConverter):

    def __init__(self, name):
        self.__light : bpy.types.SunLight = bpy.data.lights.new("light_sun_" + name, type="SUN")
        # Directional light is not supported
        self.__light.appleseed.sun_mode = "sun"
        super().__init__(name, self.__light)

    def __del__(self):
        super().__del__()

class AreaLightConverter(LightConverter):

    def __init__(self, name):
        self.__light : bpy.types.AreaLight = bpy.data.lights.new("light_area_" + name, type="AREA")
        super().__init__(name, self.__light)

    def __del__(self):
        super().__del__()

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
