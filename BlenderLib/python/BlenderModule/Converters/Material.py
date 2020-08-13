from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from ..Utils import classproperty
from .Base import DataWrapper
from .Shader import Shader

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

# Material descriptor
class MaterialData(DataWrapper):

    __defaultMat : bpy.types.Material
    __defaultMat = None

    def __init__(self, blueprintID, cpy : DataWrapper = None):
        self.__material : bpy.types.Material
        self.__material = bpy.data.materials.new(blueprintID)
        self.__material.use_nodes = True
        self.__shaderData = {}

        if cpy is None:
            self.__material.node_tree.nodes.clear()
        elif isinstance(cpy, MaterialData):
            self.CreateFromJSON(cpy.ShaderData)
        else:
            raise TypeError

        super().__init__(self.__material)

    # Override: Cleanup & remove material
    def _Cleanup(self):
        if self.__material is not None:
            bpy.data.materials.remove(self.__material)

    # Override: Get if material valid
    @property
    def Valid(self):
        return len(self.__material.node_tree.nodes) > 0

    # Get material shader data
    @property
    def ShaderData(self):
        return self.__shaderData

    # Get material block
    @property
    def Material(self) -> bpy.types.Material:
        if self.Valid:
            return self.__material
        else:
            return self.DefaultMaterial

    # Get default material block
    @classproperty
    def DefaultMaterial(cls) -> bpy.types.Material:
        # Create if necessary
        if cls.__defaultMat is None:
            # Add default shader tree to new default material
            cls.__defaultMat = bpy.data.materials.new("mat_default")
            cls.__defaultMat.use_nodes = True
            cls.__defaultMat.node_tree.nodes.clear()
            Shader.AddShader(cls.__defaultMat.node_tree, None)
        # Finally return material
        return cls.__defaultMat

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        # Store shader data
        self.__shaderData = data
        # Remove all nodes
        self.__material.node_tree.nodes.clear()
        Shader.AddShader(self.__material.node_tree, self.__shaderData)
