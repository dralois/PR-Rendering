from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

# Shader generator
class Shader(object):

    # Builds shader from json params & adds nodes to tree
    @classmethod
    def AddShader(cls, tree : bpy.types.NodeTree, data : dict):
        assert isinstance(tree, bpy.types.NodeTree)
        assert data is not None
        # Only allow to add to empty tree
        if not len(tree.nodes) > 0:
            shader = GetShader(data.get("shader", "default") if data else "default")
            # Build shader and add closure to final node
            finalNode = shader.BuildShader(tree, data.get("params", {}))
            cls.__AddClosure(finalNode, tree)

    # Add closure node and connect to closure output node
    @classmethod
    def __AddClosure(cls, finalNode : bpy.types.Node, tree : bpy.types.NodeTree):
        closure = tree.nodes.new("AppleseedasClosure2SurfaceNode")
        tree.links.new(closure.inputs[0], finalNode.outputs[0])

# Generic shader generator
class ShaderBase(object):

    # Forwarded: Build internal node network and return closure output node
    @classmethod
    def BuildShader(self, tree : bpy.types.NodeTree, data : dict) -> bpy.types.Node:
        raise NotImplementedError

# Generator for default shader
class DefaultShader(ShaderBase):

    @classmethod
    def BuildShader(self, tree : bpy.types.NodeTree, data : dict) -> bpy.types.Node:
        return tree.nodes.new("AppleseedasStandardSurfaceNode")

# Generator for module test shader
class TestShader(ShaderBase):

    @classmethod
    def BuildShader(self, tree : bpy.types.NodeTree, data : dict) -> bpy.types.Node:
        testNode = tree.nodes.new("AppleseedasModuleTestNode")
        testNode.in_color = data.get("color", (1.0,1.0,1.0))
        return testNode

# Get appropriate shader generator
def GetShader(shaderID) -> ShaderBase:
    mapping = { "default" : DefaultShader,
                "module_test" : TestShader }
    return mapping.get(shaderID, DefaultShader)
