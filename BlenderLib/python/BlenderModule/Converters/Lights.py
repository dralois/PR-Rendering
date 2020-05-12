import bpy
import mathutils

from .Base import Converter

class LightConverter(Converter):

    def __init__(self, name):
        super().__init__(name)
        self._light : bpy.types.Light = None

    def __del__(self):
        super().__del__()

    # Create general light
    def _CreateLight(self, intensity, radiance=mathutils.Color((0.8, 0.8, 0.8)), exposure=0.0, cast_indirect=True):
        # Set general parameters
        if isinstance(self._light, bpy.types.AreaLight):
            self._light.appleseed.area_intensity = intensity
            self._light.appleseed.area_color = radiance
            self._light.appleseed.area_exposure = exposure
            self._light.appleseed.cast_indirect = cast_indirect
        else:
            self._light.appleseed.radiance_multiplier = intensity
            self._light.appleseed.radiance = radiance
            self._light.appleseed.exposure = exposure
            self._light.appleseed.cast_indirect = cast_indirect
        # Store in scene
        self._CreateObject(self._light)

class PointLightConverter(LightConverter):

    def __init__(self, name):
        super().__init__(name)
        self.__fullname = "light_point_" + self._name
        self._light : bpy.types.PointLight = bpy.data.lights.new(self.__fullname, type="POINT")

    def __del__(self):
        super().__del__()

    # Create point light
    def CreatePointLight(self, intensity):
        # Point light has no special params
        self._CreateLight(intensity)

class SpotLightConverter(LightConverter):

    def __init__(self, name):
        super().__init__(name)
        self.__fullname = "light_spot_" + self._name
        self._light : bpy.types.SpotLight = bpy.data.lights.new(self.__fullname, type="SPOT")

    def __del__(self):
        super().__del__()

    # Create spot light
    def CreateSpotLight(self, intensity, angle):
        # Set spot light parameters
        self._light.spot_size = angle
        # Store in scene
        self._CreateLight(intensity)

class SunLightConverter(LightConverter):

    def __init__(self, name):
        super().__init__(name)
        self.__fullname = "light_sun_" + self._name
        self._light : bpy.types.SunLight = bpy.data.lights.new(self.__fullname, type="SUN")

    def __del__(self):
        bpy.data.lights.remove(self._light)
        super().__del__()

    # Create sun light
    def CreateSunLight(self, intensity):
        # Set sun light parameters
        self._light.appleseed.sun_mode = "sun"
        # Store in scene
        self._CreateLight(intensity)

class AreaLightConverter(LightConverter):

    def __init__(self, name):
        super().__init__(name)
        self.__fullname = "light_area_" + self._name
        self._light : bpy.types.AreaLight = bpy.data.lights.new(self.__fullname, type="AREA")

    def __del__(self):
        super().__del__()

    # Create area light
    def CreateAreaLight(self, intensity, size):
        # Set area light parameters
        self._light.shape = "DISK"
        self._light.size = size
        # Store in scene
        self._CreateLight(intensity)
