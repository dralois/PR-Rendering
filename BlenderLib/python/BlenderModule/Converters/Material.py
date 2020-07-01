from ..Utils.ClassProperty import classproperty
from ..Utils.Importer import ImportBpy
from .Base import DataWrapper
from .Shader import ShaderBase, DefaultShader

# Blender for multiprocessing
bpy = ImportBpy()

# Material descriptor
class MaterialData(DataWrapper):

    __defaultMat : bpy.types.Material
    __defaultMat = None
    __defaultShader : ShaderBase
    __defaultShader = None

    def __init__(self, name, cpy = None):
        self.__isValid = False
        self.__material : bpy.types.Material
        self.__shader : ShaderBase
        self.__shader = None

        if cpy is None:
            self.__material = bpy.data.materials.new("mat_" + name)
            self.__material.use_nodes = True
            self.__material.node_tree.nodes.clear()
        elif isinstance(cpy, MaterialData):
            self.__material = cpy.Blueprint.copy()
            self.__material.name = name
            # Recreate material from shader
            if cpy.Shader is not None:
                self.CreateFromShader(cpy.Shader.Copy())
        else:
            raise TypeError

        super().__init__(name, self.__material)

    # Override: Cleanup & remove material
    def _Cleanup(self):
        if self.__material is not None:
            bpy.data.materials.remove(self.__material)

    # Override: Get if material valid
    @property
    def Valid(self):
        return self.__isValid

    # Get material shader
    @property
    def Shader(self) -> ShaderBase:
        return self.__shader

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
            # Create, activate and clear node tree
            cls.__defaultMat = bpy.data.materials.new("mat_default")
            cls.__defaultMat.use_nodes = True
            cls.__defaultMat.node_tree.nodes.clear()
            cls.__defaultShader = DefaultShader()
            cls.__defaultShader.AddShader(cls.__defaultMat.node_tree)
        # Finally return material
        return cls.__defaultMat

    # Create material from shader
    def CreateFromShader(self, shader : ShaderBase):
        assert isinstance(shader, ShaderBase)
        # Only allow creation once
        if not self.__isValid:
            self.__shader = shader
            self.__shader.AddShader(self.__material.node_tree)
            self.__isValid = True
