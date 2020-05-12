import bpy
import bmesh
import mathutils

from .Base import Converter
from .Material import MaterialConverter

class MeshConverter(Converter):

    def __init__(self, name):
        super().__init__(name)
        self.__fullname = "mesh_" + self._name
        self.__material : bpy.types.Material = None
        self.__mesh : bpy.types.Mesh = bpy.data.meshes.new(self.__fullname)

    def __del__(self):
        super().__del__()

    # Create cube mesh
    def CreateCube(self, material):
        # Store material
        self.__material = material or MaterialConverter.DefaultMaterial()
        # Create and store cube
        bm = bmesh.new()
        bmesh.ops.create_cube(bm, size=1.0)
        bm.to_mesh(self.__mesh)
        bm.free()
        # Store in scene & assign material
        self._CreateObject(self.__mesh)
        self._obj.active_material = self.__material

    # Create & load mesh from file
    def CreateMesh(self, material, filePath):
        # Store material
        self.__material = material or MaterialConverter.DefaultMaterial()
        # TODO: Load & create mesh
        # Store in scene & assign material
        self._CreateObject(self.__mesh)
        self._obj.active_material = self.__material
