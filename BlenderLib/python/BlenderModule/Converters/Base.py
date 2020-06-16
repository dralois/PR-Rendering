from ..Utils.Importer import DoImport

# Blender for multiprocessing
bpy = DoImport()

import mathutils

class BaseConverter(object):

    def __init__(self, name, internal):
        self.__name = name
        self.__internal : bpy.types.ID = internal

    # Get name given on creation
    @property
    def Name(self):
        return self.__name

    # Get internal name
    @property
    def BlenderName(self):
        return self.__internal.name

    # Get internal object in memory
    @property
    def BlenderInternal(self) -> bpy.types.ID:
        return self.__internal

    # Get unique copy of self
    def GetCopy(self, newName):
        return BaseConverter(newName, self.__internal.copy())

class ObjectConverter(BaseConverter):

    def __init__(self, name, objData):
        # Create & add object to scene
        self.__obj : bpy.types.Object = bpy.data.objects.new(objData.name, objData)
        bpy.context.collection.objects.link(self.__obj)
        super().__init__(name, self.__obj)

    def __del__(self):
        # Removes data block & object from scene
        bpy.data.objects.remove(self.__obj)

    # Get object in memory
    @property
    def BlenderObject(self) -> bpy.types.Object:
        return self.__obj

    # Get position in scene
    @property
    def ObjectPosition(self) -> mathutils.Vector:
        return self.__obj.location

    # Set position in scene
    @ObjectPosition.setter
    def ObjectPosition(self, value):
        self.__obj.location = value

    # Get rotation of object (euler)
    @property
    def ObjectRotationEuler(self) -> mathutils.Euler:
        self.__obj.rotation_mode = "XYZ"
        return self.__obj.rotation_euler

    # Get rotation of object (quaternion)
    @property
    def ObjectRotationQuat(self) -> mathutils.Quaternion:
        self.__obj.rotation_mode = "QUATERNION"
        return self.__obj.rotation_quaternion

    # Set rotation of object (euler)
    @ObjectRotationEuler.setter
    def ObjectRotationEuler(self, value):
        self.__obj.rotation_mode = "XYZ"
        self.__obj.rotation_euler = value

    # Set rotation of object (quaternion)
    @ObjectRotationQuat.setter
    def ObjectRotationQuat(self, value):
        self.__obj.rotation_mode = "QUATERNION"
        self.__obj.rotation_quaternion = value

    # Get scale of object
    @property
    def ObjectScale(self) -> mathutils.Vector:
        return self.__obj.scale

    # Set scale of object
    @ObjectScale.setter
    def ObjectScale(self, value):
        self.__obj.scale = value
