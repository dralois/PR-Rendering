import bpy
import mathutils

from .Base import BaseConverter

class MaterialConverter(BaseConverter):

    __defaultMat : bpy.types.Material = None

    def __init__(self, name):
        self.__shaderPath = ""
        self.__material : bpy.types.Material = bpy.data.materials.new("mat_" + name)
        super().__init__(name, self.__material)

    def __del__(self):
        bpy.data.materials.remove(self.__material)

    # Get material from memory
    @property
    def Material(self) -> bpy.types.Material:
        return self.__material

    # TODO
    # Create material from shader
    def CreateFromFile(self, shaderPath):
        # Load shader from file (?)
        self.__LoadShader(shaderPath)
        # TODO
        # Create OSL node (?)
        # Create closure node
        closure = self.__SetupNodeTree(self.__material)

    # Get default material
    @classmethod
    def DefaultMaterial(cls) -> bpy.types.Material:
        # Create if necessary
        if cls.__defaultMat is None:
            # Create, activate and clear node tree
            cls.__defaultMat = bpy.data.materials.new("mat_default")
            # Create diffuse node & closure
            closure = cls.__SetupNodeTree(cls.__defaultMat)
            diffuse = cls.__defaultMat.node_tree.nodes.new('AppleseedasStandardSurfaceNode')
            # Link nodes
            cls.__defaultMat.node_tree.links.new(closure.inputs[0], diffuse.outputs[0])
        # Return the default material
        return cls.__defaultMat

    # Setup material & closure node
    @classmethod
    def __SetupNodeTree(cls, material : bpy.types.Material) -> bpy.types.Node:
         # Activate and clear node tree
        material.use_nodes = True
        material.node_tree.nodes.clear()
        # Create & return closure node
        return material.node_tree.nodes.new('AppleseedasClosure2SurfaceNode')

    # TODO
    # Load OSL shader from file
    def __LoadShader(self):
        self.__shaderPath = shaderPath
