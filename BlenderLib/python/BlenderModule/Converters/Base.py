import bpy
import mathutils

class Converter(object):

    def __init__(self, name):
        self._name = name
        self._obj : bpy.types.Object = None

    def __del__(self):
        if self._obj is not None:
            bpy.data.objects.remove(self._obj)

    # Get object in memory from bpy.data
    @property
    def BlenderObject(self):
        return self._obj

    # Get position in scene
    @property
    def Position(self):
        if self._obj is not None:
            return self._obj.location
        else:
            return mathutils.Vector(0.0, 0.0, 0.0)

    # Set position in scene
    @Position.setter
    def Position(self, value):
        if self._obj is not None:
            self._obj.location = value

    # Get rotation of object (euler)
    @property
    def RotationEuler(self):
        if self._obj is not None:
            self._obj.rotation_mode = "XYZ"
            return self._obj.rotation_euler
        else:
            return mathutils.Euler(0.0, 0.0, 0.0)

    # Get rotation of object (quaternion)
    @property
    def RotationQuat(self):
        if self._obj is not None:
            self._obj.rotation_mode = "QUATERNION"
            return self._obj.rotation_quaternion
        else:
            return mathutils.Quaternion(0.0, 0.0, 0.0, 0.0)

    # Set rotation of object (euler)
    @RotationEuler.setter
    def RotationEuler(self, value):
        if self._obj is not None:
            self._obj.rotation_mode = "XYZ"
            self._obj.rotation_euler = value

    # Set rotation of object (quaternion)
    @RotationQuat.setter
    def RotationQuat(self, value):
        if self._obj is not None:
            self._obj.rotation_mode = "QUATERNION"
            self._obj.rotation_quaternion = value

    # Get scale of object
    @property
    def Scale(self):
        if self._obj is not None:
            return self._obj.scale
        else:
            return mathutils.Vector(0.0, 0.0, 0.0)

    # Set scale of object
    @Scale.setter
    def Scale(self, value):
        if self._obj is not None:
            self._obj.scale = value

    # Create object in memory
    def _CreateObject(self, data):
        # Create object in bpy.data
        self._obj = bpy.data.objects.new("obj_" + self._name, data)
        # Add to scene
        bpy.context.collection.objects.link(self._obj)
