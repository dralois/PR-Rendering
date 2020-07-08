from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from ..Utils import FullPath
from .Base import ObjectWrapper, DataWrapper
from .Shader import Shader

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

import bmesh

# Camera descriptor
class CameraData(DataWrapper):

    def __init__(self, blueprintID, cpy : DataWrapper = None):
        self.__camera : bpy.types.Camera

        if cpy is None:
            self.__camera = bpy.data.cameras.new(blueprintID)
        elif isinstance(cpy, CameraData):
            self.__camera = cpy.Blueprint.copy()
            self.__camera.name = blueprintID
        else:
            raise TypeError

        super().__init__(self.__camera)

    # Override: Cleanup & remove camera
    def _Cleanup(self):
        if self.__camera is not None:
            bpy.data.cameras.remove(self.__camera)

    # Override: Get if camera valid
    @property
    def Valid(self):
        return self.__camera is not None

    # Get FOV [x, y]
    @property
    def CameraFOV(self):
        return self.__camera.angle_x, self.__camera.angle_y

    # Set FOV [x, y]
    @CameraFOV.setter
    def CameraFOV(self, value):
        self.__camera.angle_x = value[0]
        self.__camera.angle_y = value[1]

    # Get shift [x, y]
    @property
    def CameraShift(self):
        return self.__camera.shift_x, self.__camera.shift_y

    # Set shift [x, y]
    @CameraShift.setter
    def CameraShift(self, value):
        self.__camera.shift_x = value[0]
        self.__camera.shift_y = value[1]

    # Get near z plane distance
    @property
    def CameraNearZ(self):
        return self.__camera.appleseed.near_z

    # Set near z plane distance
    @CameraNearZ.setter
    def CameraNearZ(self, value):
        self.__camera.appleseed.near_z = value

    # Get camera frame rectangle (for current scene)
    @property
    def CameraFrame(self):
        return self.__camera.view_frame(scene = bpy.context.scene)

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        self.CameraFOV = data.get("fov", (0.6911,0.4711))
        self.CameraShift = data.get("shift", (0.0,0.0))

# Camera object in scene
class CameraInstance(ObjectWrapper):

    def __init__(self, blueprint : CameraData):
        assert isinstance(blueprint, CameraData)
        super().__init__(blueprint)
        # Store descriptor
        self.__camData : CameraData
        self.__camData = blueprint
        self.__result = ""
        self.__ppcActive = False
        # Create postprocessing effect quad
        self.__ppcMesh = bpy.data.meshes.new("mesh_camera_ppc")
        self.__CreatePPCQuad()
        # Create object & position quad
        self.__ppcObj = bpy.data.objects.new("obj_camera_ppc", self.__ppcMesh)
        self._TransformUpdate()
        # Create & setup material
        self.__ppcMat = bpy.data.materials.new("mat_camera_ppc")
        self.__ppcMat.use_nodes = True
        self.__ppcMat.node_tree.nodes.clear()
        # Bind material to quad
        self.__ppcObj.data.materials.append(self.__ppcMat)
        self.__ppcObj.material_slots[0].link = "OBJECT"
        self.__ppcObj.material_slots[0].material = self.__ppcMat

    # Get camera data
    @property
    def Blueprint(self) -> CameraData:
        return self.__camData

    # Get result file name
    @property
    def CameraResultFile(self):
        return self.__result

    # Set result file name
    @CameraResultFile.setter
    def CameraResultFile(self, value):
        self.__result = value

    # Update camera postprocessing effect
    def ChangeFullscreenEffect(self, effect):
        # If effect is None deactivate current effect
        if effect is None:
            # Unlink quad if necessary
            if self.__ppcActive:
                self.__ppcActive = False
                bpy.context.collection.objects.unlink(self.__ppcObj)
        else:
            # Otherwise add & activate it
            self.__ppcMat.node_tree.nodes.clear()
            Shader.AddPostProcessing(self.__ppcMat.node_tree, effect)
            # Link quad if necessary
            if not self.__ppcActive:
                self.__ppcActive = True
                bpy.context.collection.objects.link(self.__ppcObj)

    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        assert data is not None
        super().CreateFromJSON(data)
        self.CameraResultFile = data.get("result", "")
        self.ChangeFullscreenEffect(data.get("shader", None))

    # Override: Called when transform changes
    def _TransformUpdate(self):
        # Temporarily link for update
        if not self.__ppcActive:
            bpy.context.collection.objects.link(self.__ppcObj)
        # Set transform & translate by near plane distance
        self.__ppcObj.select_set(True)
        self.__ppcObj.matrix_world = self.ObjectTransform
        bpy.ops.transform.translate(value=(0,0,self.Blueprint.CameraNearZ), orient_type="LOCAL")
        self.__ppcObj.select_set(False)
        # Undo link for update
        if not self.__ppcActive:
            bpy.context.collection.objects.unlink(self.__ppcObj)

    # Creates quad the size of the near plane rectangle
    def __CreatePPCQuad(self):
        # Create default quad
        quadMesh = bmesh.new()
        bmesh.ops.create_grid(quadMesh, x_segments = 1, y_segments = 1, size = 1.0, calc_uvs = True)
        # Change vertices to match camera frame
        quadMesh.verts.ensure_lookup_table()
        quadMesh.verts[0].co = self.Blueprint.CameraFrame[2]
        quadMesh.verts[1].co = self.Blueprint.CameraFrame[1]
        quadMesh.verts[2].co = self.Blueprint.CameraFrame[3]
        quadMesh.verts[3].co = self.Blueprint.CameraFrame[0]
        # Change UVs to the common standard for fullscreen effects
        quadMesh.faces.ensure_lookup_table()
        uv_layer = quadMesh.loops.layers.uv.new()
        quadMesh.faces[0].loops[0][uv_layer].uv = (0,0)
        quadMesh.faces[0].loops[1][uv_layer].uv = (1,0)
        quadMesh.faces[0].loops[2][uv_layer].uv = (1,1)
        quadMesh.faces[0].loops[3][uv_layer].uv = (0,1)
        # Update postprocessing quad
        quadMesh.to_mesh(self.__ppcMesh)
        quadMesh.free()
