from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from ..Managers.TextureManager import TextureFactory

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

# Shader texture storage
__textureSystem : TextureFactory
__textureSystem = None

# Get shader texture storage
def GetTextureSystem():
    global __textureSystem
    return __textureSystem

# Set shader texture storage
def SetTextureSystem(value):
    global __textureSystem
    __textureSystem = value

# Shader generator
class Shader(object):

    # Builds shader from json params & adds nodes to tree
    @classmethod
    def AddShader(cls, tree : bpy.types.NodeTree, data : dict):
        assert isinstance(tree, bpy.types.NodeTree)
        assert data is not None
        # Only allow to add to empty tree
        if not len(tree.nodes) > 0:
            shader = GetShader(data.get("name", "default"))
            # Add textures
            for tex in data.get("textures", []):
                GetTextureSystem().AddTexture(tex.get("filePath"),
                                            tex.get("colorSpace"),
                                            tex.get("colorDepth"))
            # Build shader and add closure to final node
            finalNode = shader.BuildShader(tree, data.get("params", {}))
            cls.__AddShaderClosure(finalNode, tree)

    # Builds fullscreen post processing effect from json params & adds nodes to tree
    @classmethod
    def AddPostProcessing(cls, tree : bpy.types.NodeTree, data : dict):
        assert isinstance(tree, bpy.types.NodeTree)
        assert data is not None
        # Only allow to add to empty tree
        if not len(tree.nodes) > 0:
            shader = GetShader(data.get("name", "uv_to_color"))
            # Add textures
            for tex in data.get("textures", []):
                GetTextureSystem().AddTexture(tex.get("filePath", ""),
                                            tex.get("colorSpace", ""),
                                            tex.get("colorDepth", ""))
            # Build shader and add closure to final node
            finalNode = shader.BuildShader(tree, data.get("params", {}))
            cls.__AddEffectClosure(finalNode, tree)

    # Add shader closure node and connect to closure output node
    @classmethod
    def __AddShaderClosure(cls, finalNode : bpy.types.Node, tree : bpy.types.NodeTree):
        closure = tree.nodes.new("AppleseedasDiffuseClosureNode")
        tree.links.new(closure.inputs[0], finalNode.outputs[0])

    # Add effect closure node and connect to closure output node
    @classmethod
    def __AddEffectClosure(cls, finalNode : bpy.types.Node, tree : bpy.types.NodeTree):
        closure = tree.nodes.new("AppleseedasEffectClosureNode")
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

# Generator for default fullscreen post processing effect
class UV2Color(ShaderBase):

    @classmethod
    def BuildShader(self, tree : bpy.types.NodeTree, data : dict) -> bpy.types.Node:
        return tree.nodes.new("AppleseedasUV2ColorNode")

# Generator for diffuse 2D texture shader
class SimpleTexture(ShaderBase):

    @classmethod
    def BuildShader(self, tree : bpy.types.NodeTree, data : dict) -> bpy.types.Node:
        textureNode = tree.nodes.new("AppleseedasSimpleTextureNode")
        textureNode.in_filename = GetTextureSystem().GetTexture(data.get("filename", ""))
        return textureNode

# Get appropriate shader generator
def GetShader(shaderID) -> ShaderBase:
    # Build mapping
    mapping = { "default" : DefaultShader,
                "module_test" : TestShader,
                "uv_to_color" : UV2Color,
                "simple_texture" : SimpleTexture
    }
    # Return appropriate class
    return mapping.get(shaderID, DefaultShader)
