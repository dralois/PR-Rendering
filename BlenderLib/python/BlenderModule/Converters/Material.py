from ..Utils.ClassProperty import classproperty
from ..Utils.Importer import DoImport
from .Base import DataWrapper
from .Shader import ShaderData

# Blender for multiprocessing
bpy = DoImport()

# Material descriptor
class MaterialData(DataWrapper):

    __defaultMat : bpy.types.Material
    __defaultMat = None

    def __init__(self, name, cpy = None):
        self.__isValid = False
        self.__material : bpy.types.Material

        if cpy is None:
            self.__material = bpy.data.materials.new("mat_" + name)
        elif isinstance(cpy, MaterialData):
            self.__material = cpy.Blueprint.copy()
            self.__material.name = name
            self.__isValid = True
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

    # Get material block
    @property
    def Material(self) -> bpy.types.Material:
        if self.__isValid:
            return self.__material
        else:
            return self.DefaultMaterial

    # FIXME: Probably better to handle different
    # Get default material block
    @classproperty
    def DefaultMaterial(cls) -> bpy.types.Material:
        # Create if necessary
        if cls.__defaultMat is None:
            # Create, activate and clear node tree
            cls.__defaultMat = bpy.data.materials.new("mat_default")
            # Create diffuse node & closure
            closure = cls.__CreateClosureNode(cls.__defaultMat)
            diffuse = cls.__defaultMat.node_tree.nodes.new('AppleseedasStandardSurfaceNode')
            # Link nodes
            cls.__defaultMat.node_tree.links.new(closure.inputs[0], diffuse.outputs[0])
        # Finally return material
        return cls.__defaultMat

    # FIXME: Shader creation not fully working?
    # Create material from shader
    def CreateFromShader(self, shader : ShaderData):
        # Sanity check
        if not isinstance(shader, ShaderData):
            raise TypeError
        # Shader must be created & material not yet
        if shader.Valid and not self.__isValid:
            # Create closure node
            closure = self.__CreateClosureNode(self.__material)
            # Create OSL node
            osl = self.__CreateShaderNode(shader)
            # Assumption: Every OSL shader outputs at least one value
            self.__material.node_tree.links.new(closure.inputs[0], osl.outputs[0])
            # Material is now valid
            self.__isValid = True

    # Setup material & closure node
    @classmethod
    def __CreateClosureNode(cls, material : bpy.types.Material) -> bpy.types.Node:
         # Activate and clear node tree
        material.use_nodes = True
        material.node_tree.nodes.clear()
        # Create & return closure node
        return material.node_tree.nodes.new("AppleseedasClosure2SurfaceNode")

    # FIXME: Shader loading might not work fully
    # Create node from shader code block
    def __CreateShaderNode(self, shader : ShaderData) -> bpy.types.Node:
        # Store nodes that already exist
        nodesOld = [node.name for node in self.__material.node_tree.nodes]
        # Create script node & save shader text block
        shaderNode : bpy.types.Node = self.__material.node_tree.nodes.new("AppleseedOSLScriptBaseNode")
        shaderNode.script = shader.ShaderCode
        self.__material.node_tree.nodes.active = shaderNode
        # Create temporary mesh with material & set active
        mesh = bpy.data.meshes.new("creatorMesh")
        obj = bpy.data.objects.new("creatorObj", mesh)
        bpy.context.collection.objects.link(obj)
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.material_slot_add()
        obj.material_slots[0].material = self.__material
        # Compile shader block (-> uses material from active object)
        bpy.ops.appleseed.compile_osl_script()
        # Remove temporary object
        bpy.data.objects.remove(obj)
        # Compiler deletes script node, return the new one instead
        nodesNew = [node.name for node in self.__material.node_tree.nodes]
        createdNode = (set(nodesOld) ^ set(nodesNew)).pop()
        return self.__material.node_tree.nodes.get(createdNode)
