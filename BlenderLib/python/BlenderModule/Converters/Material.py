from ..Utils.ClassProperty import classproperty
from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from .Base import DataWrapper
from .Shader import Shader

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

# Material descriptor
class MaterialData(DataWrapper):

    __defaultMat : bpy.types.Material
    __defaultMat = None

    def __init__(self, blueprintID, cpy = None):
        self.__isValid = False
        self.__material : bpy.types.Material

        if cpy is None:
            self.__material = bpy.data.materials.new(blueprintID)
            self.__material.use_nodes = True
            self.__material.node_tree.nodes.clear()
        elif isinstance(cpy, MaterialData):
            self.__material = cpy.Blueprint.copy()
            self.__material.name = blueprintID
            self.__isValid = True
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
        return self.__isValid

    # Get material block
    @property
    def Material(self) -> bpy.types.Material:
        if self.__isValid:
            return self.__material
        else:
            return self.DefaultMaterial

    # Get default material block
    @classproperty
    def DefaultMaterial(cls) -> bpy.types.Material:
        # Create if necessary
        if cls.__defaultMat is None:
            # Add default shader tree to new default material
            cls.__defaultMat = bpy.data.materials.new("default")
            cls.__defaultMat.use_nodes = True
            cls.__defaultMat.node_tree.nodes.clear()
            Shader.AddShader(cls.__defaultMat.node_tree, None)
        # Finally return material
        return cls.__defaultMat

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        # Only allow creation once
        if not self.__isValid:
            Shader.AddShader(self.__material.node_tree, data)
            self.__isValid = True
