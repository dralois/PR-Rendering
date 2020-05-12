# Materials
Variable | Type | Description
--- | --- | ---
appleseed.mode | String | Either "surface" or "volume" material
appleseed.shader_lighting_samples | Integer | Number of samples

# Objects

## General: Transform
Variable | Type | Description
--- | --- | ---
blender.location | Vector | Position in scene
blender.rotation_euler | Vector | Rotation in euler angles
blender.rotation_quaternion | Vector | Rotation as quaternion
blender.scale | Vector | Scale of the object

## General: Visibility
Variable | Type | Description
--- | --- | ---
appleseed.enable_visibility_flags | Bool | Controls all flags
appleseed.camera_visible | Bool | Render object
appleseed.light_visible | Bool | Calculate lighting
appleseed.shadow_visible | Bool | Calculate shadows
appleseed.diffuse_visible | Bool | Recieves diffuse rays
appleseed.glossy_visible | Bool | Recieves glossy rays
appleseed.specular_visible | Bool | Recieves specular rays
appleseed.transparency_visible | Bool | Calculate transparency

## Meshes
Variable | Type | Description
--- | --- | ---
appleseed.double_sided | Bool | Use materials on both sides
appleseed.export_normals | Bool | Calculate normals
appleseed.export_uvs | Bool | Calculate UVs

## Cameras
Variable | Type | Description
--- | --- | ---
blender.shift_x | Float | Horizontal shift
blender.shift_y | Float | Vertical shift
blender.angle_x | Float | Horizontal fov (rad)
blender.angle_y | Float | Vertical fov (rad)
blender.lens | Float | Focal length (mm)
blender.sensor_width | Float | Sensor width (mm)
blender.sensor_height | Float | Sensor height (mm)
appleseed.near_z | Float | Near clip

## Lights

### General + Point Light
Variable | Type | Description
--- | --- | ---
appleseed.radiance | Vector | Light color
appleseed.radiance_multiplier | Float | Intensity
appleseed.exposure | Float | Exposure
appleseed.cast_indirect | Bool | Cast light indirect

### Spot Light:
Variable | Type | Description
--- | --- | ---
blender.spot_size | Float | Light beam angle (rad)
blender.spot_blend | Float | Light beam softness
appleseed.exposure_multiplier | Float | Exposure multiplier
appleseed.tilt_angle | Float | Light tilt angle (deg)

### Sun + Directional Light:
Variable | Type | Description
--- | --- | ---
appleseed.size_multiplier | Float | Controls shadow softness
appleseed.distance | Float | Sun distance (mio. km)
appleseed.sun_mode | String | Either "sun" or "distant"

### Area Light (completely custom!):
Variable | Type | Description
--- | --- | ---
blender.shape | String | Either "RECTANGLE", "DISK" or "SQUARE"
blender.size | Float | Size / width for rectangles
blender.size_y | Float | Height for rectangles
appleseed.area_visibility | Bool | Light mesh visibility
appleseed.area_color | Vector | Light color
appleseed.area_intensity | Float | Intensity
appleseed.area_intensity_scale | Float | Intensity multiplier
appleseed.area_exposure | Float | Exposure
