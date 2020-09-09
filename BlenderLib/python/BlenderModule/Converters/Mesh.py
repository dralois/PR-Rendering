from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from ..Utils.OutputMuter import StdMute
from ..Utils import FileExt, FullPath
from .Material import MaterialData
from .Base import DataWrapper, ObjectWrapper

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

import bmesh

# Mesh descriptor
class MeshData(DataWrapper):

    def __init__(self, blueprintID, cpy : DataWrapper = None):
        self.__isValid = False
        self.__filePath = ""
        self.__mesh : bpy.types.Mesh

        if cpy is None:
            self.__mesh = bpy.data.meshes.new(blueprintID)
        elif isinstance(cpy, MeshData):
            self.__mesh = cpy.Blueprint.copy()
            self.__mesh.name = blueprintID
            self.__isValid = True
        else:
            raise TypeError

        super().__init__(self.__mesh)

    # Override: Cleanup & remove mesh
    def _Cleanup(self):
        if self.__mesh is not None:
            # Make sure only fake user remains
            if self.__mesh.users <= 1:
                bpy.data.meshes.remove(self.__mesh)

    # Override: Get if mesh valid
    @property
    def Valid(self):
        return self.__isValid

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        # Parse filepath
        parsed = FullPath(data.get("file", ""))
        self.__isValid = (self.__filePath == parsed)
        self.__filePath = parsed
        # Load mesh or unit cube (once)
        if len(self.__filePath) > 0 and not self.__isValid:
            meshType = FileExt(self.__filePath)
            # Loading depends on extension
            if meshType == ".obj":
                self.__LoadMeshObj()
            elif meshType == ".glb":
                self.__LoadMeshGLTF()
            else:
                logger.warning(f"Mesh format {meshType} not supported!")
                self.__MakeCube()
        elif not self.__isValid:
            self.__MakeCube()

    # Create cube mesh
    def __MakeCube(self):
        # Create and flush cube
        cubeMesh = bmesh.new()
        bmesh.ops.create_cube(cubeMesh, size=1.0)
        cubeMesh.to_mesh(self.__mesh)
        cubeMesh.free()

    # Load mesh from obj file
    def __LoadMeshObj(self):
        # Load mesh to active scene & store from selection
        with StdMute():
            bpy.ops.import_scene.obj(filepath=self.__filePath, axis_forward="Y", axis_up="Z")
        # Delete potentially loaded materials
        loader : bpy.types.Object = bpy.context.selected_objects[0]
        bpy.data.batch_remove([slot.material for slot in loader.material_slots])
        # Update internals
        newMesh = loader.data
        self._Update(newMesh)
        self.__mesh = newMesh
        # Delete creator object
        bpy.data.objects.remove(loader)

    # TODO: Test if this works
    # Load mesh from glTF file
    def __LoadMeshGLTF(self):
        # Load mesh to active scene & store from selection
        with StdMute():
            bpy.ops.import_scene.gltf(filepath = self.__filePath)
        # Delete potentially loaded materials
        loader : bpy.types.Object = bpy.context.selected_objects[0]
        bpy.data.batch_remove([slot.material for slot in loader.material_slots])
        # Update internals
        newMesh = loader.data
        self._Update(newMesh)
        self.__mesh = newMesh
        # Delete creator object
        bpy.data.objects.remove(loader)

# Mesh object in scene
class MeshInstance(ObjectWrapper):

    def __init__(self, blueprint : MeshData):
        assert isinstance(blueprint, MeshData)
        super().__init__(blueprint)
        # Store descriptor
        self.__meshData : MeshData
        self.__meshData = blueprint

    # Override: Get mesh data
    @property
    def Blueprint(self) -> MeshData:
        return self.__meshData

    # Get assigned material
    @property
    def MeshMaterial(self) -> bpy.types.Material:
        return self.ObjectInstance.active_material

    # Set assigned material
    @MeshMaterial.setter
    def MeshMaterial(self, value : MaterialData):
        assert isinstance(value, MaterialData)
        # Set material active
        if value.Material is not None:
            # Make slot if there isn't one
            if len(self.ObjectInstance.material_slots) == 0:
                self.ObjectInstance.data.materials.append(value.Material)
            # Bind material to object
            self.ObjectInstance.material_slots[0].link = "OBJECT"
            self.ObjectInstance.material_slots[0].material = value.Material

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        super().CreateFromJSON(data)
        # Mesh unique name
        self.ObjectInstance.name = f'obj_{data.get("objectID", "?")}_{self.Blueprint.Name}'
        # Mesh visibility settings
        self.ObjectInstance.appleseed.camera_visible = not data.get("indirect", False)
        self.ObjectInstance.appleseed.shadow_visible = not data.get("indirect", False)
