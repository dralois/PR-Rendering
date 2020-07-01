from ..Utils.Importer import ImportBpy
from ..Utils.Logger import get_logger
from .Material import MaterialData
from .Base import DataWrapper, ObjectWrapper

# Blender for multiprocessing
bpy = ImportBpy()

import os
import bmesh
import mathutils

logger = get_logger()

# Mesh descriptor
class MeshData(DataWrapper):

    def __init__(self, name, cpy = None):
        self.__isValid = False
        self.__mesh : bpy.types.Mesh

        if cpy is None:
            self.__mesh = bpy.data.meshes.new("mesh_" + name)
        elif isinstance(cpy, MeshData):
            self.__mesh = cpy.Blueprint.copy()
            self.__mesh.name = name
            self.__isValid = True
        else:
            raise TypeError

        super().__init__(name, self.__mesh)

    # Override: Cleanup & remove mesh
    def _Cleanup(self):
        if self.__mesh is not None:
            bpy.data.meshes.remove(self.__mesh)

    # Override: Get if mesh valid
    @property
    def Valid(self):
        return self.__isValid

    # Create mesh from file
    def CreateFromFile(self, filePath):
        # Load mesh or unit cube
        if len(filePath) > 0 and not self.__isValid:
            meshType = os.path.splitext(filePath)[1]
            # Loading depends on extension
            if meshType == ".obj":
                self.__LoadMeshObj(filePath)
            else:
                logger.warning(f"Mesh format {meshType} not supported!")
        elif not self.__isValid:
            self.__MakeCube()
        # Mesh is now valid
        self.__isValid = True

    # Create cube mesh
    def __MakeCube(self):
        # Create and store cube
        cubeMesh = bmesh.new()
        bmesh.ops.create_cube(cubeMesh, size=1.0)
        cubeMesh.to_mesh(self.__mesh)
        cubeMesh.free()

    # Load mesh from obj file
    def __LoadMeshObj(self, filePath):
        self.__filePath = filePath
        # Load mesh to active scene & store from selection
        bpy.ops.import_scene.obj(filepath = self.__filePath)
        loader : bpy.types.Object = bpy.context.selected_objects[0]
        # Delete potentially loaded materials
        bpy.data.batch_remove([slot.material for slot in loader.material_slots])
        # Update internals
        newMesh = loader.data
        self._Update(newMesh)
        self.__mesh = newMesh
        # Delete creator object
        bpy.data.objects.remove(loader)

# Mesh object in scene
class MeshInstance(ObjectWrapper):

    def __init__(self, name, data : MeshData):
        super().__init__(name, data)
        # Store descriptor
        self.__meshData : MeshData
        self.__meshData = data
        # Create material slot
        bpy.context.view_layer.objects.active = self.ObjectInstance
        bpy.ops.object.material_slot_add()

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
            self.ObjectInstance.material_slots[0].material = value.Material
            self.ObjectInstance.active_material = value.Material
