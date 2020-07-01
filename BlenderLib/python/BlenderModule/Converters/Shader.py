from ..Utils.Importer import ImportBpy

# Blender for multiprocessing
bpy = ImportBpy()

# Generic shader wrapper
class ShaderBase(object):

    def __init__(self, name):
        self.__name = name
        self.__finalNode : bpy.types.Node
        self.__finalNode = None
        self.__isValid = False

    # Get if shader has been initialized
    @property
    def Valid(self):
        return self.__isValid

    # Get shader name
    @property
    def Name(self):
        return self.__name

    # Get final node before closure
    @property
    def FinalNode(self):
        assert self.__isValid
        return self.__finalNode

    # Creates copy of shader and returns it
    def Copy(self):
        return type(self)(self.__name)

    # Adds and builds shader to node tree
    def AddShader(self, tree : bpy.types.NodeTree):
        assert isinstance(tree, bpy.types.NodeTree)
        # Only allow to add once
        if not self.__isValid:
            # Build internal network and add closure
            self.__finalNode = self._BuildInternal(tree)
            self.__AddClosure(self.__finalNode, tree)
            self.__isValid = True

    # Override: Build internal node network and return closure output node
    def _BuildInternal(self, tree : bpy.types.NodeTree) -> bpy.types.Node:
        raise NotImplementedError

    # Add closure node and connect to closure output node
    @staticmethod
    def __AddClosure(finalNode : bpy.types.Node, tree : bpy.types.NodeTree):
        closure = tree.nodes.new("AppleseedasClosure2SurfaceNode")
        tree.links.new(closure.inputs[0], finalNode.outputs[0])

# Wrapper for default shader
class DefaultShader(ShaderBase):

    def _BuildInternal(self, tree : bpy.types.NodeTree) -> bpy.types.Node:
        return tree.nodes.new("AppleseedasStandardSurfaceNode")

# Wrapper for module test shader
class TestShader(ShaderBase):

    def SetColor(self, color):
        self.FinalNode.in_color = color

    def _BuildInternal(self, tree : bpy.types.NodeTree) -> bpy.types.Node:
        return tree.nodes.new("AppleseedasModuleTestNode")

# Get appropriate shader class
def GetShader(shaderID) -> ShaderBase:
    mapping = { "default" : DefaultShader,
                "asModuleTest" : TestShader }
    return mapping.get(shaderID, DefaultShader)
