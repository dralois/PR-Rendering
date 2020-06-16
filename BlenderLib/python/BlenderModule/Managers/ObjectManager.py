from ..Converters import Base, Camera, Lights, Material, Mesh

from typing import List, Dict, Generic, TypeVar

T = TypeVar("T", Camera.CameraConverter, Lights.LightConverter, Material.MaterialConverter, Mesh.MeshConverter)

class NoOriginalException(Exception):
    pass

class ObjectStorage(Generic[T]):

    def __init__(self):
        self.__storage : Dict[str, T]
        self.__storage = {}
        self.__original : Dict[str, T]
        self.__original = {}
        self.__mapping : Dict[str, str]
        self.__mapping = {}

    def __del__(self):
        del self.__storage
        del self.__original
        del self.__mapping

    # Get all matching objects
    def GetByName(self, name) -> List[T]:
        matches = []
        # Find all objects with given name
        for obj in self.__storage.values():
            if obj.Name == name:
                matches.append(obj)
        # Return them
        return matches

    # Remove all matching objects
    def RemoveByName(self, name):
        matches = []
        # Find all objects with given name
        for bpyName, obj in self.__storage.items():
            if obj.Name == name:
                matches.append(obj)
        # Remove them
        for obj in matches:
            self._Remove(obj)

    # Attempt to store & return a new copy
    def TryMakeCopy(self, objId, name) -> T:
        if objId in self.__original:
            return self._Store(self.__original[objId].GetCopy(name))
        else:
            raise NoOriginalException()

    # Store original & return it
    def MakeOriginal(self, objId, obj) -> T:
        return self.__Store(obj, objId)

    # Save object in storage
    def __Store(self, obj : T, objId = None) -> T:
        bpyName = obj.BlenderName
        # Potentially make original
        if objId is not None:
            self.__original[objId] = obj
            self.__mapping[bpyName] = objId
        # Always store & return object
        self.__storage[bpyName] = obj
        return self.__storage[bpyName]

    # Remove object from storage
    def __Remove(self, obj : T):
        bpyName = obj.BlenderName
        # Remove stored object
        if bpyName in self.__storage:
            del self.__storage[bpyName]
        # Potentially remove from copying
        if bpyName in self.__mapping:
            del self.__original[self.__mapping[bpyName]]
            del self.__mapping[bpyName]

class MaterialFactory(ObjectStorage[Material.MaterialConverter]):

    def __init__(self):
        super().__init__()

    def __del__(self):
        super().__del__()

    # Create material with params
    def CreateMaterial(self, name, path) -> Material.MaterialConverter:
        # Always try to copy original first
        try:
            return self.TryMakeCopy(path, name)
        except NoOriginalException:
            # If no original, create and return that
            newMat = Material.MaterialConverter(name)
            newMat.CreateFromFile(path)
            return self.MakeOriginal(path, newMat)

class MeshFactory(ObjectStorage[Mesh.MeshConverter]):

    def __init__(self):
        super().__init__()

    def __del__(self):
        super().__del__()

    # Create mesh with params
    def CreateMesh(self, name, path, material) -> Mesh.MeshConverter:
        # Always try to copy original first
        try:
            cpy = self.TryMakeCopy(path, name)
            cpy.MeshMaterial = material
            return cpy
        except NoOriginalException:
            # If no original, create and return that
            newMesh = Mesh.MeshConverter(name)
            newMesh.CreateFromFile(path)
            newMesh.MeshMaterial = material
            return self.MakeOriginal(path, newMesh)

class LightFactory(ObjectStorage[Lights.LightConverter]):

    def __init__(self):
        super().__init__()

    def __del__(self):
        super().__del__()

    # Create light with params
    def CreateLight(self, name, type) -> Lights.LightConverter:
        # Always try to copy original first
        try:
            return self.TryMakeCopy(type, name)
        except NoOriginalException:
            # If no original, create and return that
            if type == "AREA":
                newLight = Lights.AreaLightConverter(name)
            elif type == "SPOT":
                newLight = Lights.SpotLightConverter(name)
            elif type == "SUN":
                newLight = Lights.SunLightConverter(name)
            else:
                newLight = Lights.PointLightConverter(name)
            # Original light is based on type
            return self.MakeOriginal(type, newLight)
