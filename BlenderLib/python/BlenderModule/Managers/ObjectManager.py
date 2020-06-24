from ..Converters import Base, Camera, Lights, Material, Mesh, Shader

from typing import List, Dict, Generic, TypeVar

T = TypeVar("T",
            Camera.CameraData,
            Lights.GenericLightData,
            Material.MaterialData,
            Mesh.MeshData,
            Shader.ShaderData
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
        self.__uniques.append(unique)
        return unique

    # Remove unique blueprint copy from storage
    def RemoveUnique(self, unique : T):
        if unique in self.__uniques:
            self.__uniques.remove(unique)

# Generic storage of actual instances in a scene
class InstanceStorage(Generic[V, T]):

    def __init__(self):
        self.__blueprints : BlueprintStorage[T]
        self.__blueprints = BlueprintStorage[T]()
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

    # Get blueprint factory
    @property
    def _Blueprints(self) -> BlueprintStorage[T]:
        return self.__blueprints

    # Adds an instance to storage
    def _AddInstance(self, instance : V) -> V:
        if instance is not None:
            self.__instances.append(instance)
            return instance
        else:
            raise ValueError

    # Removes an instance from storage
    def _RemoveInstance(self, instance : V):
        if instance in self.__instances:
            self._Blueprints.RemoveUnique(instance)
            self.__instances.remove(instance)
        else:
            raise ValueError

# Shader storage & factory
class ShaderFactory(BlueprintStorage[Shader.ShaderData]):

    # Override: Make & return unique shader
    def GetUnique(self, blueprint : Shader.ShaderData) -> Shader.ShaderData:
        uniqueShader = Shader.ShaderData(blueprint.BlueprintID + self.NextUniqueID, blueprint)
        return self.AddUnique(uniqueShader)

    # Get OSL shader blueprint
    def GetShaderBlueprint(self, path, uniqueShader = False) -> Shader.ShaderData:
        try:
            # Unique or instanced shader
            if uniqueShader:
                return self.GetUnique(self.GetBlueprint(path))
            else:
                return self.GetBlueprint(path)
        except NoBlueprintException:
            # If no blueprint, create and return it
            newShader = Shader.ShaderData(GetFileName(path))
            newShader.CreateFromFile(path)
            # Unique or instanced shader
            if uniqueShader:
                return self.GetUnique(self.AddBlueprint(path, newShader))
            else:
                return self.AddBlueprint(path, newShader)

# Material storage & factory
class MaterialFactory(BlueprintStorage[Material.MaterialData]):

    def __init__(self):
        super().__init__()
        self.__shaders : ShaderFactory
        self.__shaders = ShaderFactory()

    def __del__(self):
        super().__del__()
        del self.__shaders

    # Override: Make & return unique shader
    def GetUnique(self, blueprint : Material.MaterialData) -> Material.MaterialData:
        uniqueMat = Material.MaterialData(blueprint.BlueprintID + self.NextUniqueID, blueprint)
        return self.AddUnique(uniqueMat)

    # Get material blueprint
    def GetMaterialBlueprint(self, shaderPath, uniqueMat = False) -> Material.MaterialData:
        try:
            # Unique or instanced material
            if uniqueMat:
                return self.GetUnique(self.GetBlueprint(shaderPath))
            else:
                return self.GetBlueprint(shaderPath)
        except NoBlueprintException:
            # If no blueprint, create it
            newMat = Material.MaterialData(GetFileName(shaderPath))
            # Fetch required OSL shader & create material
            shader = self.__shaders.GetShaderBlueprint(shaderPath)
            newMat.CreateFromShader(shader)
            # Unique or instanced material
            if uniqueMat:
                return self.GetUnique(self.AddBlueprint(shaderPath, newMat))
            else:
                return self.AddBlueprint(shaderPath, newMat)

# Mesh storage & factory & instancer
class MeshFactory(InstanceStorage[Mesh.MeshInstance, Mesh.MeshData]):

    def __init__(self):
        super().__init__()
        self.__materials : MaterialFactory
        self.__materials = MaterialFactory()

    def __del__(self):
        super().__del__()
        del self.__materials

    # Create mesh instance from blueprint
    def AddMeshInstance(self, meshName, meshBlueprint : Mesh.MeshData, shaderPath):
        # Sanity check
        if not isinstance(meshBlueprint, Mesh.MeshData):
            raise TypeError
        # Fetch required material blueprint
        mat = self.__materials.GetMaterialBlueprint(shaderPath, False)
        mesh = Mesh.MeshInstance(meshName, meshBlueprint)
        mesh.MeshMaterial = mat
        return self._AddInstance(mesh)

    # Get mesh blueprint
    def GetMeshBlueprint(self, meshPath) -> Mesh.MeshData:
        try:
            return self._Blueprints.GetBlueprint(meshPath)
        except NoBlueprintException:
            # If no blueprint, create mesh
            newMesh = Mesh.MeshData(GetFileName(meshPath))
            newMesh.CreateFromFile(meshPath)
            # Add & return blueprint
            return self._Blueprints.AddBlueprint(meshPath, newMesh)

# Light storage & factory & instancer
class LightFactory(InstanceStorage[Lights.GenericLightInstance, Lights.GenericLightData]):

    # Create light instance from blueprint
    def AddLightInstance(self, lightName, lightBlueprint : Lights.GenericLightData) -> Lights.GenericLightInstance:
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
            raise TypeError
        # Add and return it
        return self._AddInstance(light)

    # Get light blueprint
    def GetLightBlueprint(self, type) -> Lights.GenericLightData:
        # Always try to return a copied light blueprint first
        try:
            if type == "AREA":
                return self._Blueprints.AddUnique(Lights.AreaLightData(type, self._Blueprints.GetBlueprint(type)))
            elif type == "SPOT":
                return self._Blueprints.AddUnique(Lights.SpotLightData(type, self._Blueprints.GetBlueprint(type)))
            elif type == "SUN":
                return self._Blueprints.AddUnique(Lights.SunLightData(type, self._Blueprints.GetBlueprint(type)))
            elif type == "POINT":
                return self._Blueprints.AddUnique(Lights.PointLightData(type, self._Blueprints.GetBlueprint(type)))
            else:
                raise ValueError
        except NoBlueprintException:
            # If no blueprint, create one and return copied version
            if type == "AREA":
                newLight = Lights.AreaLightData(type)
                blueprint = self._Blueprints.AddBlueprint(type, newLight)
                return self._Blueprints.AddUnique(Lights.AreaLightData(type, blueprint))
            elif type == "SPOT":
                newLight = Lights.SpotLightData(type)
                blueprint = self._Blueprints.AddBlueprint(type, newLight)
                return self._Blueprints.AddUnique(Lights.SpotLightData(type, blueprint))
            elif type == "SUN":
                newLight = Lights.SunLightData(type)
                blueprint = self._Blueprints.AddBlueprint(type, newLight)
                return self._Blueprints.AddUnique(Lights.SunLightData(type, blueprint))
            elif type == "POINT":
                newLight = Lights.PointLightData(type)
                blueprint = self._Blueprints.AddBlueprint(type, newLight)
                return self._Blueprints.AddUnique(Lights.PointLightData(type, blueprint))
            else:
                raise ValueError
