from typing import List, Dict, Generic, TypeVar

from ..Converters import Base, Camera, Lights, Material, Mesh

T = TypeVar("T", Camera.CameraConverter, Lights.LightConverter, Material.MaterialConverter, Mesh.MeshConverter)

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

    # Save object in storage
    def _Store(self, obj : T, objId = None):
        bpyName = obj.BlenderName
        # Potentially store for copying
        if objId is not None:
            self.__original[objId] = obj
            self.__mapping[bpyName] = objId
        # Always save in storage
        self.__storage[bpyName] = obj

    # Try to to create from original
    def _Copy(self, objId, name) -> T:
        # If original exists
        if objId in self.__original:
            # Make & store copy
            cpy = self.__original[objId].GetCopy(name)
            self._Store(cpy)
            # Return it
            return cpy
        else:
            return None

    # Remove object from storage
    def _Remove(self, obj : T):
        bpyName = obj.BlenderName
        # Remove stored object
        if bpyName in self.__storage:
            del self.__storage.pop(bpyName)
        # Potentially remove from copying
        if bpyName in self.__mapping:
            del self.__original.pop(self.__mapping[bpyName])
            del self.__mapping[bpyName]

class MaterialFactory(ObjectStorage[Material.MaterialConverter]):

    def __init__(self):
        super().__init__()

    def __del__(self):
        super().__del__()

    # Create material with params
    def CreateMaterial(self, name, path) -> Material.MaterialConverter:
        # Try to copy first
        newMat = self._Copy(path, name)
        # If not possible, create original
        if newMat is None:
            newMat = Material.MaterialConverter(name)
            newMat.CreateFromFile(path)
            self._Store(newMat, path)
        # Finally return object
        return newMat

class MeshFactory(ObjectStorage[Mesh.MeshConverter]):

    def __init__(self):
        super().__init__()

    def __del__(self):
        super().__del__()

    # Create mesh with params
    def CreateMesh(self, name, path, material) -> Mesh.MeshConverter:
        # Try to copy first
        newMesh = self._Copy(path, name)
        # If not possible, create original
        if newMesh is None:
            newMesh = Mesh.MeshConverter(name)
            newMesh.CreateFromFile(path)
            newMesh.MeshMaterial = material
            self._Store(newMesh, path)
        # Finally return object
        return newMesh

class LightFactory(ObjectStorage[Lights.LightConverter]):

    def __init__(self):
        super().__init__()

    def __del__(self):
        super().__del__()

    # Create light with params
    def CreateLight(self, name, type) -> Lights.LightConverter:
        # Try to copy first
        newLight = self._Copy(type, name)
        # If not possible, create original
        if newLight is None:
            if type == "AREA":
                newLight = Lights.AreaLightConverter(name)
            elif type == "SPOT"
                newLight = Lights.SpotLightConverter(name)
            elif type == "SUN"
                newLight = Lights.SunLightConverter(name)
            else:
                newLight = Lights.PointLightConverter(name)
            # Original light is based on type
            self._Store(newLight, type)
        # Finally return object
        return newLight
