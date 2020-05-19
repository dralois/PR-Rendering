import os
import bpy
import bmesh
import mathutils

# Appleseed init
try:
    bpy.ops.preferences.addon_refresh()
    bpy.ops.preferences.addon_enable(module = 'blenderseed')
except:
    bpy.ops.preferences.addon_install(overwrite = False, filepath = os.getcwd() + '\\blenderseed.zip')
    bpy.ops.preferences.addon_enable(module = 'blenderseed')

# Remove defaults
bpy.data.batch_remove([obj.data for obj in bpy.data.objects])
bpy.data.batch_remove([mat for mat in bpy.data.materials])

# Create a cube
mesh : bpy.types.Mesh = bpy.data.meshes.new('cube')
bm = bmesh.new()
bmesh.ops.create_cube(bm, size=1.0)
bm.to_mesh(mesh)
bm.free()
ob : bpy.types.Object = bpy.data.objects.new('cube', mesh)

# Create an appleseed material
mat : bpy.types.Material = bpy.data.materials.new(name="Diffuse")
mat.use_nodes = True
mat.node_tree.nodes.clear()
diffuse = mat.node_tree.nodes.new('AppleseedasStandardSurfaceNode')
closure = mat.node_tree.nodes.new('AppleseedasClosure2SurfaceNode')
mat.node_tree.links.new(closure.inputs[0], diffuse.outputs[0])
ob.active_material = mat

# Create a light
light_data : bpy.types.PointLight = bpy.data.lights.new('light', type='POINT')
light_data.appleseed.radiance_multiplier = 100
light = bpy.data.objects.new('light', light_data)
light.location = mathutils.Vector((3, -4.2, 5))

# Create the camera
cam_data :bpy.types.Camera = bpy.data.cameras.new('camera')
cam = bpy.data.objects.new('camera', cam_data)
cam.location = mathutils.Vector((6, -3, 5))
cam.rotation_euler = mathutils.Euler((0.9, 0.0, 1.1))

# Link objects to currently active collection
bpy.context.collection.objects.link(ob)
bpy.context.collection.objects.link(light)
bpy.context.collection.objects.link(cam)

# Render settings
bpy.context.scene.render.image_settings.file_format = 'PNG'
bpy.context.scene.render.filepath = os.getcwd() + '/output.png'
bpy.context.scene.render.resolution_x = 1280
bpy.context.scene.render.resolution_y = 720
bpy.context.scene.render.engine = 'APPLESEED_RENDER'
bpy.context.scene.camera = cam

# Render & save
bpy.ops.render.render(write_still = True)
