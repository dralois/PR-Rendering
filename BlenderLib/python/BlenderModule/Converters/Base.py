from ..Utils.Importer import ImportBpy

# Blender for multiprocessing
bpy = ImportBpy()

import mathutils

# Base object class
class BaseWrapper(object):

    def __init__(self, name):
        self.__name = name

    def __del__(self):
        self._Cleanup()

    # Get name given on creation
    @property
    def Name(self):
        return self.__name

    # Override: Get if blueprint is valid
    @property
    def Valid(self):
        raise NotImplementedError

    # Override: Get blueprint name
    @property
    def BlueprintID(self):
        raise NotImplementedError

    # Override: Get blueprint data
    @property
    def Blueprint(self):
        raise NotImplementedError

    # Override: Cleanup internal data
    def _Cleanup(self):
        raise NotImplementedError

    # Override: Update internal data
    def _Update(self, newData):
        raise NotImplementedError

# Arbitrary object without transform
class DataWrapper(BaseWrapper):

    def __init__(self, name, data : bpy.types.ID):
        super().__init__(name)
        assert isinstance(data, bpy.types.ID)
        # Store data block
        self.__data : bpy.types.ID
        self.__data = data
        # Make sure the data block isn't deleted
        self.__data.use_fake_user = True

    # Forwarded: Get if valid
    @property
    def Valid(self):
        raise NotImplementedError

    # Override: Get internal name
    @property
    def BlueprintID(self):
        return self.__data.name

    # Override: Get internal data
    @property
    def Blueprint(self):
        return self.__data

    # Forwarded: Cleanup internal data
    def _Cleanup(self):
        raise NotImplementedError

    # Override: Cleanup old & update internal data
    def _Update(self, newData : bpy.types.ID):
        oldName = self.BlueprintID
        self._Cleanup()
        self.__data = newData
        self.__data.name = oldName

# Object with transform in scene
class ObjectWrapper(BaseWrapper):

    def __init__(self, name, data : DataWrapper):
        super().__init__(name)
        assert isinstance(data, DataWrapper)
        # Store object data
        self.__data = data
        # Create & add object to scene
        self.__obj : bpy.types.Object
        self.__obj = bpy.data.objects.new(name, data.Blueprint)
        bpy.context.collection.objects.link(self.__obj)

    # Override: Get if instance valid
    @property
    def Valid(self):
        return self.__data.Valid()

    # Override: Get blueprint name
    @property
    def BlueprintID(self):
        return self.__obj.name

    # Forwarded: Get blueprint
    @property
    def Blueprint(self):
        raise NotImplementedError

    # Get object instance
    @property
    def ObjectInstance(self) -> bpy.types.Object:
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

    # Override: Erase object & remove from scene
    def _Cleanup(self):
        if self.__obj is not None:
            bpy.data.objects.remove(self.__obj)

    # Override: Update object data (keeps transform)
    def _Update(self, newData : DataWrapper):
        self.__data = newData
        self.__obj.data = self.__data.Blueprint
