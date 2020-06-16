from ..Utils.Importer import DoImport

# Blender for multiprocessing
bpy = DoImport()

import bmesh
import mathutils

from .Base import ObjectConverter
from .Material import MaterialConverter

class MeshConverter(ObjectConverter):

    def __init__(self, name):
        self.__filePath = ""
        self.__mesh : bpy.types.Mesh = bpy.data.meshes.new("mesh_" + name)
        super().__init__(name, self.__mesh)
        # Assign default material on creation
        self.BlenderObject.active_material = MaterialConverter.DefaultMaterial()

    def __del__(self):
        super().__del__()

    # Get assigned material
    @property
    def MeshMaterial(self) -> bpy.types.Material:
        return self.BlenderObject.active_material

    # Set assigned material
    @MeshMaterial.setter
    def MeshMaterial(self, value):
        if value is not None:
            self.BlenderObject.active_material = value

    # TODO
    # Create mesh from file
    def CreateFromFile(self, filePath):
        # Load mesh or unit cube
        if filePath == "":
            self.__MakeCube()
        else:
            self.__LoadMesh(filePath)

    # Create cube mesh
    def __MakeCube(self):
        # Create and store cube
        cubeMesh = bmesh.new()
        bmesh.ops.create_cube(cubeMesh, size=1.0)
        cubeMesh.to_mesh(self.__mesh)
        cubeMesh.free()

    # TODO
    # Load mesh from (obj) file
    def __LoadMesh(self, filePath):
        self.__filePath = filePath
