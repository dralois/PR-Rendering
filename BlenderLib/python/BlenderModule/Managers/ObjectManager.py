from ..Converters import Base, Camera, Lights, Material, Mesh, Shader
from ..Utils.Logger import GetLogger
from ..Utils import FileName

logger = GetLogger()

from typing import List, Dict, Generic, TypeVar

T = TypeVar("T",
            Camera.CameraData,
            Lights.GenericLightData,
            Material.MaterialData,
            Mesh.MeshData
)

V = TypeVar("V",
            Camera.CameraInstance,
            Lights.GenericLightInstance,
            Mesh.MeshInstance
)

# Generic blueprint storage
class BlueprintStorage(Generic[T]):

    def __init__(self):
        self.__blueprints : Dict[str, T]
        self.__blueprints = {}
        self.__uniques : List[T]
        self.__uniques = []
        self.__uniqueID = 0

    def __del__(self):
        del self.__blueprints
        del self.__uniques

    # Get blueprint by json data
    def GetBlueprint(self, data : dict, unique = False) -> T:
        # Parse specific identifier from data
        blueprintID = self._GetBlueprintID(data)
        # Make blueprint if it doesnt exist
        if blueprintID not in self.__blueprints:
            blueprint = self._MakeBlueprint(data)
            self.__blueprints[blueprintID] = blueprint
            logger.info(f"{self}: Adding blueprint {blueprintID}")
        # Return unique or existing blueprint
        if unique:
            return self._AddUnique(self.__blueprints[blueprintID])
        else:
            return self.__blueprints[blueprintID]

    # Forwarded: Get blueprint identifier
    def _GetBlueprintID(self, data : dict):
        raise NotImplementedError

    # Forwarded: Make new blueprint from json data
    def _MakeBlueprint(self, data : dict) -> T:
        raise NotImplementedError

    # Store unique blueprint copy
    def _AddUnique(self, blueprint : T) -> T:
        logger.debug(f"{self}: Adding unique blueprint {blueprint.Name}")
        # Make new version from blueprint & add & return it
        unique = type(blueprint)(blueprint.Name + self.__NextUniqueID(), blueprint)
        self.__uniques.append(unique)
        return unique

    # Remove unique blueprint copy from storage
    def _RemoveUnique(self, unique : T):
        logger.debug(f"{self}: Removing unique blueprint {unique.Name}")
        if unique in self.__uniques:
            self.__uniques.remove(unique)

    # Get a unique identifier string
    def __NextUniqueID(self) -> str:
        self.__uniqueID += 1
        return "_{:03d}".format(self.__uniqueID)

# Generic storage of actual instances in a scene
class InstanceStorage(Generic[V, T], BlueprintStorage[T]):

    def __init__(self):
        super().__init__()
        self.__instances : List[V]
        self.__instances = []
        self.__hashCache = {}
        self.__updateCache = []

    def __del__(self):
        del self.__instances
        del self.__hashCache
        del self.__updateCache
        super().__del__()

    # Get instance by json data
    def GetInstance(self, data : dict, unique = False) -> V:
        assert data is not None
        # Get blueprint
        blueprint = self.GetBlueprint(data, unique)
        # Make & return new instance
        instance = self._MakeInstance(data, blueprint)
        return self._AddInstance(instance)

    # Update an existing instance from json data
    def UpdateInstance(self, data : dict) -> V:
        assert data is not None
        # Create expected hash & mark as up to date
        search = self._BuildHash(data)
        self.__updateCache.append(search)
        # Return updated instance if found
        if search in self.__hashCache:
            return self._UpdateInstance(data, self.__hashCache[search])
        else:
            return None

    # Removes all instances that are not up to date
    def RemoveStaleInstances(self):
        staleInstances = []
        # Find stale instances
        for instance in self.__instances:
            if self._BuildHash(instance) not in self.__updateCache:
                staleInstances.append(instance)
        # Delete them
        for instance in staleInstances:
            logger.info(f"Removing stale {instance}")
            self._RemoveInstance(instance)
        # Clear up to date cache
        self.__updateCache.clear()

    # Forwarded: Make new instance from json data & blueprint
    def _MakeInstance(self, data : dict, blueprint : T) -> V:
        raise NotImplementedError

    # Forwarded: Build hash value for instance / json
    def _BuildHash(self, toHash):
        if isinstance(toHash, Base.BaseWrapper):
            return toHash.Name
        else:
            return hash(toHash)

    # Forwarded: Updates existing instance with new json data
    def _UpdateInstance(self, data : dict, instance : V) -> V:
        raise NotImplementedError

    # Adds an instance to storage
    def _AddInstance(self, instance : V) -> V:
        logger.debug(f"{self}: Adding new instance of {instance.Name}")
        if instance is not None:
            self.__instances.append(instance)
            hashed = self._BuildHash(instance)
            self.__hashCache[hashed] = instance
            self.__updateCache.append(hashed)
            return instance
        else:
            raise ValueError

    # Removes an instance from storage
    def _RemoveInstance(self, instance : V):
        logger.debug(f"{self}: Removing instance {instance.Name}")
        if instance in self.__instances:
            self._RemoveUnique(instance)
            self.__instances.remove(instance)
            hashed = self._BuildHash(instance)
            del self.__hashCache[hashed]
            if hashed in self.__updateCache:
                self.__updateCache.remove(hashed)
        else:
            raise ValueError

# Material storage & factory
class MaterialFactory(BlueprintStorage[Material.MaterialData]):

    # Override: Get blueprint identifier
    def _GetBlueprintID(self, data : dict):
        assert data is not None
        return "mat_" + data.get("name", "default")

    # Override: Make new blueprint from json data
    def _MakeBlueprint(self, data : dict) -> Material.MaterialData:
        assert data is not None
        # Get identifier from json data
        blueprintID = self._GetBlueprintID(data)
        # Make & return new blueprint
        newMat = Material.MaterialData(blueprintID)
        newMat.CreateFromJSON(data)
        return newMat

# Mesh storage & factory & instancer
class MeshFactory(InstanceStorage[Mesh.MeshInstance, Mesh.MeshData]):

    def __init__(self):
        super().__init__()
        self.__materials : MaterialFactory
        self.__materials = MaterialFactory()

    def __del__(self):
        super().__del__()
        del self.__materials

    # Override: Get blueprint identifier
    def _GetBlueprintID(self, data : dict):
        assert data is not None
        return f'mesh_{FileName(data.get("file", "?"))}'

    # Override: Make new blueprint from json data
    def _MakeBlueprint(self, data : dict) -> Mesh.MeshData:
        assert data is not None
        # Get identifier from json data
        blueprintID = self._GetBlueprintID(data)
        # Make & return new blueprint
        newMesh = Mesh.MeshData(blueprintID)
        newMesh.CreateFromJSON(data)
        return newMesh

    # Override: Make new instance from json data & blueprint
    def _MakeInstance(self, data : dict, blueprint : Mesh.MaterialData) -> Mesh.MeshInstance:
        assert isinstance(blueprint, Mesh.MeshData)
        assert data is not None
        # Always get unique material
        shader = data.get("shader", {})
        uniqueMat = self.__materials.GetBlueprint(shader, True)
        uniqueMat.CreateFromJSON(shader)
        # Make instance & return it
        meshInstance = Mesh.MeshInstance(blueprint)
        meshInstance.CreateFromJSON(data)
        meshInstance.MeshMaterial = uniqueMat
        return meshInstance

    # Override: Build hash value for instance / json
    def _BuildHash(self, toHash):
        if isinstance(toHash, dict):
            if "objectID" in toHash:
                return toHash["objectID"]
            else:
                raise ValueError
        elif isinstance(toHash, Mesh.MeshInstance):
            return toHash.ObjectID
        else:
            raise ValueError

    # Override: Updates existing instance with new json data
    def _UpdateInstance(self, data : dict, instance : V) -> V:
        assert isinstance(instance, Mesh.MeshInstance)
        assert data is not None
        # Always get unique material
        shader = data.get("shader", {})
        uniqueMat = self.__materials.GetBlueprint(shader, True)
        uniqueMat.CreateFromJSON(shader)
        # Update instance & return it
        instance.CreateFromJSON(data)
        instance.MeshMaterial = uniqueMat
        return instance

# Camera storage & factory & instancer
class CameraFactory(InstanceStorage[Camera.CameraInstance, Camera.CameraData]):

    # Override: Get instance by json data
    def GetInstance(self, data : dict, unique = True) -> Camera.CameraInstance:
        # Camera should be unique
        return super().GetInstance(data, unique)

    # Override: Get blueprint identifier
    def _GetBlueprintID(self, data : dict):
        assert data is not None
        return f'cam_{FileName(data.get("resultFile", "?"))}'

    # Override: Make new blueprint from json data
    def _MakeBlueprint(self, data : dict) -> Camera.CameraData:
        assert data is not None
        # Get identifier from json data
        blueprintID = self._GetBlueprintID(data)
        # Make & return new blueprint
        newCam = Camera.CameraData(blueprintID)
        newCam.CreateFromJSON(data)
        return newCam

    # Override: Make new instance from json data & blueprint
    def _MakeInstance(self, data : dict, blueprint : Camera.CameraData) -> Camera.CameraInstance:
        assert isinstance(blueprint, Camera.CameraData)
        assert data is not None
        # Make instance & return it
        camInstance = Camera.CameraInstance(blueprint)
        camInstance.CreateFromJSON(data)
        return camInstance

# Light storage & factory & instancer
class LightFactory(InstanceStorage[Lights.GenericLightInstance, Lights.GenericLightData]):

    # Override: Get instance by json data
    def GetInstance(self, data : dict, unique = True) -> Lights.GenericLightInstance:
        # Lights should be unique
        return super().GetInstance(data, unique)

    # Override: Get blueprint identifier
    def _GetBlueprintID(self, data : dict):
        assert data is not None
        return f'light_{data.get("type", "?")}'

    # Override: Make new blueprint from json data
    def _MakeBlueprint(self, data : dict) -> Lights.GenericLightData:
        assert data is not None
        # Get identifier from json data
        blueprintID = self._GetBlueprintID(data)
        # Make new blueprint depending on type
        if "AREA" in blueprintID:
            newLight = Lights.AreaLightData(blueprintID)
        elif "SPOT" in blueprintID:
            newLight = Lights.SpotLightData(blueprintID)
        elif "SUN" in blueprintID:
            newLight = Lights.SunLightData(blueprintID)
        elif "POINT" in blueprintID:
            newLight = Lights.PointLightData(blueprintID)
        else:
            raise ValueError
        # Create & return it
        newLight.CreateFromJSON(data)
        return newLight

    # Override: Make new instance from json data & blueprint
    def _MakeInstance(self, data : dict, blueprint : Lights.GenericLightData) -> Lights.GenericLightInstance:
        assert isinstance(blueprint, Lights.GenericLightData)
        assert data is not None
        # Make instance & return it
        if isinstance(blueprint, Lights.AreaLightData):
            lightInstance = Lights.AreaLightInstance(blueprint)
        elif isinstance(blueprint, Lights.SpotLightData):
            lightInstance = Lights.SpotLightInstance(blueprint)
        elif isinstance(blueprint, Lights.SpotLightData):
            lightInstance = Lights.SunLightInstance(blueprint)
        elif isinstance(blueprint, Lights.PointLightData):
            lightInstance = Lights.PointLightInstance(blueprint)
        else:
            raise ValueError
        # Create & return it
        lightInstance.CreateFromJSON(data)
        return lightInstance
