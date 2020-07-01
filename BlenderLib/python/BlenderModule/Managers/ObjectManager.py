from ..Converters import Base, Camera, Lights, Material, Mesh, Shader
from ..Utils.Logger import get_logger

from typing import List, Dict, Generic, TypeVar

logger = get_logger()

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

# Get file name from path
def GetFileName(path):
    from os.path import split, splitext
    file = split(path)[1]
    return splitext(file)[0]

# Thrown if blueprint not created yet
class NoBlueprintException(Exception):
    pass

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

    # Get blueprint by ID
    def GetBlueprint(self, blueprintID) -> T:
        # Return only if blueprint exists
        if blueprintID not in self.__blueprints:
            raise NoBlueprintException
        else:
            return self.__blueprints[blueprintID]

    # Add new blueprint by ID
    def AddBlueprint(self, blueprintID, blueprint : T) -> T:
        logger.info(f"Adding blueprint {blueprintID} ({type(blueprint)})")
        # Don't override blueprints
        if blueprintID not in self.__blueprints:
            self.__blueprints[blueprintID] = blueprint
        # Return created or existing blueprint
        return self.__blueprints[blueprintID]

    # Get a unique identifier string
    @property
    def NextUniqueID(self) -> str:
        self.__uniqueID += 1
        return "_{:03d}".format(self.__uniqueID)

    # Override: Make & return unique copy
    def GetUnique(self, blueprint : T) -> T:
        raise NotImplementedError

    # Store unique blueprint copy
    def AddUnique(self, unique : T) -> T:
        logger.info(f"Adding unique blueprint of {unique.BlueprintID} ({type(unique)})")
        self.__uniques.append(unique)
        return unique

    # Remove unique blueprint copy from storage
    def RemoveUnique(self, unique : T):
        logger.info(f"Removing unique blueprint of {unique.BlueprintID} ({type(unique)})")
        if unique in self.__uniques:
            self.__uniques.remove(unique)

# Generic storage of actual instances in a scene
class InstanceStorage(Generic[V, T]):

    def __init__(self):
        self.__blueprints : BlueprintStorage[T]
        self.__blueprints = BlueprintStorage[T]()
        self.__blueprints.GetUnique = self.GetUnique
        self.__instances : List[V]
        self.__instances =  []

    def __del__(self):
        del self.__blueprints
        del self.__instances

    # Get all matching instances
    def GetInstances(self, name) -> List[V]:
        matches = []
        # Find all instances with given name
        for obj in self.__instances.values():
            if obj.Name == name:
                matches.append(obj)
        # Return them
        return matches

    # Remove all matching instances
    def RemoveInstances(self, name):
        matches = []
        # Find all instances with given name
        for obj in self.__instances.values():
            if obj.Name == name:
                matches.append(obj)
        # Remove them
        for obj in matches:
            self._RemoveInstance(obj)

    # Override: Make & return unique copy
    def GetUnique(self, blueprint : T) -> T:
        raise NotImplementedError

    # Get blueprint factory
    @property
    def _Blueprints(self) -> BlueprintStorage[T]:
        return self.__blueprints

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
            self._Blueprints.RemoveUnique(instance)
            self.__instances.remove(instance)
        else:
            raise ValueError

# Material storage & factory
class MaterialFactory(BlueprintStorage[Material.MaterialData]):

    # Override: Make & return unique shader
    def GetUnique(self, blueprint : Material.MaterialData) -> Material.MaterialData:
        # Sanity check
        assert isinstance(blueprint, Material.MaterialData)
        # Create unique blueprint
        uniqueMat = Material.MaterialData(blueprint.BlueprintID + self.NextUniqueID, blueprint)
        return self.AddUnique(uniqueMat)

    # Get material blueprint
    def GetMaterialBlueprint(self, shader : Shader.ShaderBase, uniqueMat = True) -> Material.MaterialData:
        assert isinstance(shader, Shader.ShaderBase)
        # Potentially create blueprint first
        try:
            # Unique or instanced material
            if uniqueMat:
                return self.GetUnique(self.GetBlueprint(shader.Name))
            else:
                return self.GetBlueprint(shader.Name)
        except NoBlueprintException:
            # No blueprint, create it from shader
            newMat = Material.MaterialData(shader.Name)
            newMat.CreateFromShader(shader)
            # Unique or instanced material
            if uniqueMat:
                return self.GetUnique(self.AddBlueprint(shader.Name, newMat))
            else:
                return self.AddBlueprint(shader.Name, newMat)

# Mesh storage & factory & instancer
class MeshFactory(InstanceStorage[Mesh.MeshInstance, Mesh.MeshData]):

    def __init__(self):
        super().__init__()
        self.__materials : MaterialFactory
        self.__materials = MaterialFactory()

    def __del__(self):
        super().__del__()
        del self.__materials

    # Override: Make & return unique mesh
    def GetUnique(self, blueprint : Mesh.MeshData) -> Mesh.MeshData:
        assert isinstance(blueprint, Mesh.MeshData)
        # Create unique blueprint
        uniqueMesh = Mesh.MeshData(blueprint.BlueprintID + self._Blueprints.NextUniqueID, blueprint)
        return self._Blueprints.AddUnique(uniqueMesh)

    # Create mesh instance from blueprint
    def AddMeshInstance(self, meshName, meshBlueprint : Mesh.MeshData, shader : Shader.ShaderBase):
        assert isinstance(meshBlueprint, Mesh.MeshData)
        assert isinstance(shader, Shader.ShaderBase)
        # Fetch required material blueprint
        mat = self.__materials.GetMaterialBlueprint(shader)
        mesh = Mesh.MeshInstance(meshName, meshBlueprint)
        mesh.MeshMaterial = mat
        return self._AddInstance(mesh)

    # Get mesh blueprint
    def GetMeshBlueprint(self, meshPath, uniqueMesh = True) -> Mesh.MeshData:
        # Potentially create blueprint first
        try:
            # Unique or instanced mesh
            if uniqueMesh:
                return self._Blueprints.GetUnique(self._Blueprints.GetBlueprint(meshPath))
            else:
                return self._Blueprints.GetBlueprint(meshPath)
        except NoBlueprintException:
            # No blueprint, create mesh
            newMesh = Mesh.MeshData(GetFileName(meshPath))
            newMesh.CreateFromFile(meshPath)
            # Unique or instanced mesh
            if uniqueMesh:
                return self._Blueprints.GetUnique(self._Blueprints.AddBlueprint(meshPath, newMesh))
            else:
                return self._Blueprints.AddBlueprint(meshPath, newMesh)

# Light storage & factory & instancer
class LightFactory(InstanceStorage[Lights.GenericLightInstance, Lights.GenericLightData]):

    # Override: Make & return unique light
    def GetUnique(self, blueprint : Lights.GenericLightData) -> Lights.GenericLightData:
        assert isinstance(blueprint, Lights.GenericLightData)
        # Sanity check & make unique light
        if isinstance(blueprint, Lights.AreaLightData):
            uniqueLight = Lights.AreaLightData(blueprint.BlueprintID + self._Blueprints.NextUniqueID, blueprint)
        elif isinstance(blueprint, Lights.SpotLightData):
            uniqueLight = Lights.SpotLightData(blueprint.BlueprintID + self._Blueprints.NextUniqueID, blueprint)
        elif isinstance(blueprint, Lights.SunLightData):
            uniqueLight = Lights.SunLightData(blueprint.BlueprintID + self._Blueprints.NextUniqueID, blueprint)
        elif isinstance(blueprint, Lights.PointLightData):
            uniqueLight = Lights.PointLightData(blueprint.BlueprintID + self._Blueprints.NextUniqueID, blueprint)
        else:
            raise ValueError
        # Return unique light
        return self._Blueprints.AddUnique(uniqueLight)

    # Create light instance from blueprint
    def AddLightInstance(self, lightName, lightBlueprint : Lights.GenericLightData) -> Lights.GenericLightInstance:
        assert isinstance(lightBlueprint, Lights.GenericLightData)
        # Make instance depending on light type
        if isinstance(lightBlueprint, Lights.AreaLightData):
            light = Lights.AreaLightInstance(lightName, lightBlueprint)
        elif isinstance(lightBlueprint, Lights.SpotLightData):
            light = Lights.SpotLightInstance(lightName, lightBlueprint)
        elif isinstance(lightBlueprint, Lights.SunLightData):
            light = Lights.SunLightInstance(lightName, lightBlueprint)
        elif isinstance(lightBlueprint, Lights.PointLightData):
            light = Lights.PointLightInstance(lightName, lightBlueprint)
        else:
            raise ValueError
        # Add and return it
        return self._AddInstance(light)

    # Get light blueprint
    def GetLightBlueprint(self, type) -> Lights.GenericLightData:
        assert type in ["AREA", "SPOT", "SUN", "POINT"]
        # Potentially create blueprint first
        try:
            return self._Blueprints.GetUnique(self._Blueprints.GetBlueprint(type))
        except NoBlueprintException:
            # If no blueprint, create one
            if type == "AREA":
                newLight = Lights.AreaLightData(type)
            elif type == "SPOT":
                newLight = Lights.SpotLightData(type)
            elif type == "SUN":
                newLight = Lights.SunLightData(type)
            elif type == "POINT":
                newLight = Lights.PointLightData(type)
            else:
                raise ValueError
            # Return unique blueprint
            return self._Blueprints.GetUnique(self._Blueprints.AddBlueprint(type, newLight))
