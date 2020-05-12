import bpy
import mathutils

class MaterialConverter(object):

    # Can be used if no other material available
    __defaultMat = None

    def __init__(self, name):
        self.__fullname = "mat_" + name
        self.__material : bpy.types.Material = bpy.data.materials.new(self.__fullname)

    def __del__(self):
        bpy.data.materials.remove(self.__material)

    # Get material in memory from bpy.data
    @property
    def Material(self):
        return self.__material

    # Get the default material
    @classmethod
    def DefaultMaterial(cls):
        # Create if necessary
        if cls.__defaultMat is None:
            # Create, activate and clear node tree
            cls.__defaultMat = bpy.data.materials.new("mat_default")
            cls.__defaultMat.use_nodes = True
            cls.__defaultMat.node_tree.nodes.clear()
            # Create diffuse node & closure
            diffuse = cls.__defaultMat.node_tree.nodes.new('AppleseedasStandardSurfaceNode')
            closure = cls.__defaultMat.node_tree.nodes.new('AppleseedasClosure2SurfaceNode')
            # Link nodes
            cls.__defaultMat.node_tree.links.new(closure.inputs[0], diffuse.outputs[0])
        # Return the default material
        return cls.__defaultMat

    # Create material from file
    def CreateMaterial(self, matPath):
         # Activate and clear node tree
        self.__material.use_nodes = True
        self.__material.node_tree.nodes.clear()
        # TODO: Create nodes & closure
        closure = self.__material.node_tree.nodes.new('AppleseedasClosure2SurfaceNode')
        # TODO: Link nodes

    # Load OSL shader from file
    def LoadShader(self, shaderPath):
        return
