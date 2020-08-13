from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

import mathutils

# Base object class
class BaseWrapper(object):

    def __del__(self):
        self._Cleanup()

    # Forwarded: Get if blueprint is valid
    @property
    def Valid(self):
        raise NotImplementedError

    # Forwarded: Get blueprint name
    @property
    def Name(self):
        raise NotImplementedError

    # Forwarded: Get blueprint data
    @property
    def Blueprint(self):
        raise NotImplementedError

    # Forwarded: Create from json data
    def CreateFromJSON(self, data : dict):
        raise NotImplementedError

    # Forwarded: Cleanup internal data
    def _Cleanup(self):
        raise NotImplementedError

    # Forwarded: Update internal data
    def _Update(self, newData):
        raise NotImplementedError

# Arbitrary object without transform
class DataWrapper(BaseWrapper):

    def __init__(self, data : bpy.types.ID):
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
    def Name(self):
        return self.__data.name

    # Override: Get internal data
    @property
    def Blueprint(self):
        return self.__data

    # Forwarded: Create from json data
    def CreateFromJSON(self, data : dict):
        raise NotImplementedError

    # Forwarded: Cleanup internal data
    def _Cleanup(self):
        raise NotImplementedError

    # Override: Cleanup old & update internal data
    def _Update(self, newData : bpy.types.ID):
        oldName = self.Name
        self._Cleanup()
        self.__data = newData
        self.__data.name = oldName

# Object with transform in scene
class ObjectWrapper(BaseWrapper):

    def __init__(self, data : DataWrapper):
        assert isinstance(data, DataWrapper)
        # Store object data
        self.__id = -1
        self.__data = data
        # Create & add object to scene
        self.__obj : bpy.types.Object
        self.__obj = bpy.data.objects.new("obj_" + data.Name, data.Blueprint)
        bpy.context.collection.objects.link(self.__obj)

    # Override: Get if instance valid
    @property
    def Valid(self):
        return self.__data.Valid()

    # Override: Get object name
    @property
    def Name(self):
        return self.__obj.name

    # Forwarded: Get blueprint
    @property
    def Blueprint(self):
        raise NotImplementedError

    # Get object instance id
    @property
    def ObjectID(self):
        return self.__id

    # Get object instance data
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

    # Get rotation of object
    @property
    def ObjectRotation(self) -> mathutils.Quaternion:
        return self.__obj.rotation_quaternion

    # Set rotation of object
    @ObjectRotation.setter
    def ObjectRotation(self, value):
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

    # Forwarded & Override: Create from json data
    def CreateFromJSON(self, data : dict):
        # Parse identifier
        self.__id = data.get("objectID", -1)
        # Parse transform
        self.ObjectPosition = mathutils.Vector(data.get("position", (0.0, 0.0, 0.0)))
        self.ObjectRotation = mathutils.Quaternion(data.get("rotation", (1.0, 0.0, 0.0, 0.0)))
        self.ObjectScale = mathutils.Vector(data.get("scale", (1.0, 1.0, 1.0)))
        # Parse specific data
        self.__data.CreateFromJSON(data)

    # Override: Erase object & remove from scene
    def _Cleanup(self):
        if self.__obj is not None:
            bpy.data.objects.remove(self.__obj)

    # Override: Update object data (keeps transform)
    def _Update(self, newData : DataWrapper):
        self.__data = newData
        self.__obj.data = self.__data.Blueprint
