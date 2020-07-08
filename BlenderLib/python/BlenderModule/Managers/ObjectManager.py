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
            logger.info(f"Adding blueprint {blueprintID} ({type(blueprint)})")
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
        logger.info(f"Adding unique blueprint of {blueprint.BlueprintID} ({type(blueprint)})")
        # Make new version from blueprint & add & return it
        unique = type(blueprint)(blueprint.BlueprintID + self.__NextUniqueID(), blueprint)
        self.__uniques.append(unique)
        return unique

    # Remove unique blueprint copy from storage
    def _RemoveUnique(self, unique : T):
        logger.info(f"Removing unique blueprint of {unique.BlueprintID} ({type(unique)})")
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

    def __del__(self):
        del self.__instances
        super().__del__()

    # Get instance by json data
    def GetInstance(self, data : dict, unique = False) -> V:
        assert data is not None
        # Get blueprint
        blueprint = self.GetBlueprint(data, unique)
        # Make & return new instance
        instance = self._MakeInstance(data, blueprint)
        return self._AddInstance(instance)

    # Forwarded: Make new instance from json data & blueprint
    def _MakeInstance(self, data : dict, blueprint : T) -> V:
        raise NotImplementedError

    # Adds an instance to storage
    def _AddInstance(self, instance : V) -> V:
        logger.info(f"Adding new instance of {instance.BlueprintID} ({type(instance)})")
        if instance is not None:
            self.__instances.append(instance)
            return instance
        else:
            raise ValueError

    # Removes an instance from storage
    def _RemoveInstance(self, instance : V):
        logger.info(f"Removing instance {instance.BlueprintID} ({type(instance)})")
        if instance in self.__instances:
            self._RemoveUnique(instance)
            self.__instances.remove(instance)
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
        return "mesh_" + FileName(data.get("file", ""))

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

# Light storage & factory & instancer
class LightFactory(InstanceStorage[Lights.GenericLightInstance, Lights.GenericLightData]):

    # Override: Get instance by json data
    def GetInstance(self, data : dict, unique = True) -> V:
        # Lights should be unique
        return super().GetInstance(data, unique)

    # Override: Get blueprint identifier
    def _GetBlueprintID(self, data : dict):
        assert data is not None
        return "light_" + data.get("type", "NONE")

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
        if "AREA" in blueprint.BlueprintID:
            lightInstance = Lights.AreaLightInstance(blueprint)
        elif "SPOT" in blueprint.BlueprintID:
            lightInstance = Lights.SpotLightInstance(blueprint)
        elif "SUN" in blueprint.BlueprintID:
            lightInstance = Lights.SunLightInstance(blueprint)
        elif "POINT" in blueprint.BlueprintID:
            lightInstance = Lights.PointLightInstance(blueprint)
        else:
            raise ValueError
        # Create & return it
        lightInstance.CreateFromJSON(data)
        return lightInstance
