from .Utils.ImgProcUtils import *
from .Utils.MeshUtils import *
from .Utils.SolverUtils import *
from .Utils.ShaderUtils import *

from shutil import rmtree
from plotoptix import NpOptiX
from plotoptix.materials import m_flat
from plotoptix.enums import ChannelDepth, ChannelOrder, Camera

import numpy as np
np.seterr(invalid='ignore')
import cv2

import math
import os
import threading
import random
import pickle
import json

# 1) Import all images, inverse gamma correct them (y = 2.2)

# Original RGB, gamma corrected RGB, masked RGB, gradients, extrinsics matrix, eye pos
frames = {}
mesh = None
intr_c = (0, 0, 0, 0, 0, 0)
intr_o = (0, 0, 0, 0, 0, 0)

gamma_correct = True
store_debug_mesh = False
store_debug_panorama = False
randomize_samples = True
solve_max_verts = 200000

test_path = ".\\HDRLib\\Test\\1"
temp_path = ""

# TODO: For each scene
with os.scandir(test_path) as dir:
    for direntry in dir:
        # Frames dir?
        if direntry.is_dir() and direntry.name == "rgbd":
            # Required to store intermediate results
            temp_path = os.path.join(direntry.path, os.pardir, "temp")
            os.makedirs(temp_path, exist_ok=True)
            # Load info file
            info = {}
            for line in open(os.path.join(direntry.path, "_info.txt"), "r").readlines():
                entry = line.split("=")
                info[entry[0].strip()] = entry[1].strip()
            # Load camera intrinsics
            intr_loaded = load_intr(info["m_calibrationColorIntrinsic"])[:3, :3]
            w, h = int(info["m_colorWidth"]), int(info["m_colorHeight"])
            fx, fy = intr_loaded[0][0], intr_loaded[1][1]
            ox, oy = intr_loaded[0][2], intr_loaded[1][2]
            cx = ((ox - (w / 2)) / max(w, h))
            cy = ((oy - (h / 2)) / max(w, h))
            intr_c = (fx, fy, cx, cy, w, h)
            intr_o = (fx, fy, ox, oy, w, h)
            # Loop frames
            for i in range(0, max(1, int("".join(filter(str.isdigit, os.listdir(direntry.path)[-2]))))):
                # Read frame, depth, pose
                base = "frame-{0:06d}".format(i)
                org = cv2.cvtColor(
                        cv2.imread(os.path.join(direntry.path, base + ".color.jpg"), cv2.IMREAD_COLOR),
                        cv2.COLOR_BGR2RGB)
                rgb = org if not gamma_correct else adjust_gamma(org, 2.2)
                depth = cv2.resize(
                            cv2.imread(os.path.join(direntry.path, base + ".depth.pgm"), cv2.IMREAD_ANYDEPTH),
                            (org.shape[1], org.shape[0]))
                # Create a mask where the depth is unknown
                mask = np.repeat(depth != 0, 3, 1).reshape(rgb.shape[0], rgb.shape[1], 3)
                # Create the corresponding masked rgb image
                rgbmask = np.dstack((np.ma.array(rgb, mask=mask).filled(0), depth == 0))
                # The left and right side never has depth -> Remove it
                rgbmask[:,:92] = 0
                rgbmask[:,-90:] = 0
                # Read the pose & build a gradient map (masking edges)
                extr = load_extr(os.path.join(direntry.path, base + ".pose.txt"))
                grads = gradient_map(rgb)
                # Extract camera position
                eye = np.reshape(extr[0:3, 3:4], (3))
                # Convert camera2world -> world2camera
                flip = np.array([[ 1,  0,  0,  0],
                                [ 0, -1,  0,  0],
                                [ 0,  0, -1,  0],
                                [ 0,  0,  0,  1]])
                extr = np.linalg.inv(np.matmul(extr, flip))
                # Store
                frames[base] = (org, rgb, rgbmask, grads, extr, eye)
        # Load mesh
        elif direntry.is_file() and direntry.name == "mesh.refined.v2.obj":
            mesh = load_mesh(direntry.path, "obj")

# 2) Project onto scene geometry
# 2.1) Raytrace all vertices -> all cameras, locate pixel if not occluded

# Used for target data texture
size = math.ceil(math.sqrt(len(mesh.vertices)))

# Necessary to only process one frame at a time
renderDone = threading.Event()

# Locates hit pixels
def RenderDone(rt: NpOptiX):
    # Next frame can be processed
    renderDone.set()

# Load farmework
rt = NpOptiX(on_rt_accum_done=RenderDone, width=size, height=size, start_now=False)
rt.set_param(min_accumulation_step=1, max_accumulation_frames=1)

# Build ray targets texture
targets = np.full((size*size, 4), -1.0, dtype=np.float32)
targets[:mesh.vertices.shape[0],:3] = mesh.vertices[:,:3]
targets = targets.reshape(size, size, 4)
rt.set_texture_2d("target", targets)

# Create camera & mesh
rt.setup_material("flat", m_flat)
rt.setup_camera("cam", cam_type=Camera.CustomProjXYZ, textures=["target"])
rt.set_mesh("mesh", mesh.vertices, mesh.faces, mat="flat")

# Start raytracing
rt.start()

# Stores unobstructed pixels
vertexPixels = {fr : np.full((len(mesh.vertices), 3), np.NaN) for fr in frames}

# Try loading intermediate results
try:
    with open(os.path.join(temp_path, "VertexPixels.pkl"), mode="r+b") as stream:
        vertexPixels = pickle.load(stream)
except OSError:
    # Create compute shader
    with ShaderManager(".\\HDRLib\\Shader\\VertexHitmap.comp") as compute:

        # Mesh buffer (constant): struct{uvec4, vec4[3]}
        faces = np.int32(np.hstack((mesh.faces, np.zeros((mesh.faces.shape[0], 1), dtype=np.int64))))
        triangles = np.float32(np.dstack((mesh.triangles, np.ones((mesh.triangles.shape[0], mesh.triangles.shape[1], 1), dtype=np.float64))))
        meshdata = np.concatenate((faces.view("float32"), triangles.reshape(-1, 12)), axis=1)
        compute.SetData(2, meshdata)

        # Target buffer (constant): uvec4
        targetdata = targets.reshape((-1, 4))
        compute.SetData(3, targetdata)

        # Raytrace all frames
        for frame, vals in frames.items():
            # Load current frame values & update camera position
            _, rgb, _, grads, extr, eye_pos = vals
            rt.update_camera("cam", eye=eye_pos)
            rt.refresh_scene()

            # One frame at a time
            renderDone.wait()

            # Load hit targets (fid: hit face indices, pid: hit vertex index)
            fid = rt._geo_id[:,:,1].reshape(rt._height * rt._width)
            pid = rt._geo_id[:,:,0].reshape(rt._height * rt._width)
            pid &= 0xC0000000
            pid >>= 30

            # Hit targets buffer: struct{uint, uint}
            hitData = np.vstack((fid, pid)).transpose()
            compute.SetData(1, hitData)

            # Camera variables: mat4, vec2, vec2, vec2
            camVars = np.concatenate((np.float32(extr.transpose().flatten()), np.array(intr_c, dtype=np.float32)))
            compute.SetData(4, camVars)

            # Image data: uvec4 - RGB + gradient (0/1)
            imageData = np.uint32(np.dstack((rgb, np.uint8(grads)))).reshape(-1, 4)
            compute.SetData(5, imageData)

            # Output buffer: uvec2
            output = np.full((len(mesh.vertices), 4), -1, dtype=np.int32)
            compute.SetData(6, output)

            # Calculate in shader
            compute.StartCompute(math.ceil(hitData.shape[0] / 128))
            result = compute.GetData(6)[:,:3]
            vertexPixels[frame] = np.where(result >= 0.0, result, result * np.nan)

            # Next frame
            renderDone.clear()

        # Store intermediate results
        with open(os.path.join(temp_path, "VertexPixels.pkl"), mode="x+b") as stream:
            pickle.dump(vertexPixels, stream)

# 2.2) Solve for exposure & radiance

# Select random subset of max 200.000 verts
vert_idx = ([i for i in range(min(len(mesh.vertices), solve_max_verts))] if not randomize_samples else
    random.sample(range(min(len(mesh.vertices), solve_max_verts)), min(len(mesh.vertices), solve_max_verts)))
selection = np.array([np.take(pixels, vert_idx, axis=0) for _, pixels in vertexPixels.items()]).transpose(1, 0, 2)

# Try loading intermediate results
try:
    with open(os.path.join(temp_path, "Exposures.pkl"), mode="r+b") as stream:
        exposure = pickle.load(stream)
except OSError:
    # Solve for exposure
    exposure = SolveExposure(selection)
    # Store intermediate results
    with open(os.path.join(temp_path, "Exposures.pkl"), mode="x+b") as stream:
        pickle.dump(exposure, stream)
    # Store solved exposure (convert to EV -> logarithmic) for future use
    with open(os.path.join(temp_path, os.pardir, "exposures.json"), mode="w") as stream:
        exposures = dict(zip(frames.keys(), np.log2(exposure)[:,0].tolist()))
        json.dump(exposures, stream, indent=1)

# 3) Reproject radiance

# Stores weighted radiance samples
vertexRadiance = {fr : np.full((len(mesh.vertices), 3), np.NaN) for fr in frames}

# Try loading intermediate results
try:
    with open(os.path.join(temp_path, "Reprojection.pkl"), mode="r+b") as stream:
        vertexRadiance = pickle.load(stream)
except OSError:
    # Create compute shader
    with ShaderManager(".\\HDRLib\\Shader\\RadianceReproject.comp") as compute:

        # Mesh buffer (constant): struct{uvec4, vec4, vec4[3]}
        faces = np.int32(np.hstack((mesh.faces, np.zeros((mesh.faces.shape[0], 1), dtype=np.int64))))
        triangles = np.float32(np.dstack((mesh.triangles, np.ones((mesh.triangles.shape[0], mesh.triangles.shape[1], 1), dtype=np.float64))))
        normals = np.float32(np.hstack((mesh.face_normals, np.zeros((mesh.faces.shape[0], 1), dtype=np.float64))))
        meshdata = np.concatenate((faces.view("float32"), normals, triangles.reshape(-1, 12)), axis=1)
        compute.SetData(2, meshdata)

        # Target buffer (constant): uvec4
        targetdata = targets.reshape((-1, 4))
        compute.SetData(3, targetdata)

        # Raytrace all frames again
        for frame, vals in frames.items():
            # Load current frame values & update camera position
            org, _, _, _, extr, eye_pos = vals
            rt.update_camera("cam", eye=eye_pos)
            rt.refresh_scene()

            # Calculate optical axis & load exposure
            eye = np.float32(np.append(eye_pos, 1.0))
            oj = np.float32(np.matmul(np.linalg.inv(extr), np.array([0.0, 0.0, -1.0, 0.0])))
            ex = np.float32(np.append(exposure[int("".join(filter(str.isdigit, frame)))], 0.0))

            # One frame at a time
            renderDone.wait()

            # Load hit targets (fid: hit face indices, pid: hit vertex index)
            fid = rt._geo_id[:,:,1].reshape(rt._height * rt._width)
            pid = rt._geo_id[:,:,0].reshape(rt._height * rt._width)
            pid &= 0xC0000000
            pid >>= 30

            # Hit targets buffer: struct{uint, uint}
            hitData = np.vstack((fid, pid)).transpose()
            compute.SetData(1, hitData)

            # Camera variables: mat4, vec2, vec2, vec2, padding, vec4, vec4, vec4
            camVars = np.concatenate((np.float32(extr.transpose().flatten()),
                np.array(intr_c, dtype=np.float32), np.zeros((2), dtype=np.float32), eye, oj, ex))
            compute.SetData(4, camVars)

            # Original image data: uvec4
            orgData = np.uint32(np.dstack((org, np.zeros((org.shape[0], org.shape[1], 1), dtype=np.uint8)))).reshape(-1, 4)
            compute.SetData(5, orgData)

            # Output buffer: vec4
            output = np.full((len(mesh.vertices), 4), -1, dtype=np.float32)
            compute.SetData(6, output)

            # Calculate in shader
            compute.StartCompute(math.ceil(hitData.shape[0] / 128))
            result = compute.GetData(6)
            vertexRadiance[frame] = np.where(result >= 0.0, result, result * np.nan)

            # Next frame
            renderDone.clear()

        # Store intermediate results
        with open(os.path.join(temp_path, "Reprojection.pkl"), mode="x+b") as stream:
            pickle.dump(vertexRadiance, stream)

# Generate median vertex color samples
samples = np.array([vertices for _, vertices in vertexRadiance.items()]).transpose((1, 0, 2))
weightedsum = np.nansum(samples[:,:,:3] * samples[:,:,3:4], axis=1)
weightsum = np.nansum(samples[:,:,3:4], axis=1)
samples = weightedsum / weightsum
samples = np.where(np.isnan(samples), np.zeros_like(samples), samples / 255.0)

# Update raytracing mesh
rt.set_mesh("mesh", mesh.vertices, mesh.faces, mat="flat", c=samples)

# Potentially store "HDR" mesh
if store_debug_mesh:
    # Store & output reprojected colors
    set_vertex_colors(mesh, samples * 255.0)
    store_mesh(os.path.join(temp_path, os.pardir, "hdr_mesh.glb"), mesh)

# 4) Render HDR panorama

# Determine the best camera position of the frames (to avoid being stuck in geometry)
center = mesh.bounding_box.centroid
eye_points = np.array([eye for _, _, _, _, _, eye in frames.values()])
best_candidate = eye_points[np.linalg.norm(np.abs(eye_points - center), ord=2, axis=1).argmin()]
cam_target = best_candidate + np.array([0, -1, 0])

# Setup raytracer for 360* panorama capture
rt.resize(7768, 3884)
rt.set_param(min_accumulation_step=100, max_accumulation_frames=300)
rt.setup_camera("pano", cam_type=Camera.Panoramic, eye=best_candidate, target=cam_target, up=[0, 0, 1])
rt.set_current_camera("pano")
rt.refresh_scene()

# Wait for convergence
renderDone.clear()
renderDone.wait()

# Get HDR panorama (color + depth)
pan_hdr = rt.get_rt_output(ChannelDepth.Bps32, ChannelOrder.BGR)
pan_depth = np.ma.array(rt._hit_pos[:,:,3:4], mask=rt._hit_pos[:,:,3:4]>1e+9)

# 5) Detect light sources
# 5.1) Compute illuminance

# Calculate raw luminance
lum = raw_luminance(pan_hdr)

# Create compute shader
with ShaderManager(".\\HDRLib\\Shader\\Luminance.comp") as compute:
    # Set data
    compute.SetData(1, lum.flatten())
    compute.SetData(2, np.array([lum.shape[1], lum.shape[0]]))
    output = np.zeros_like(lum, dtype=np.float32)
    compute.SetData(3, output.flatten())
    # Compute illuminance
    compute.StartCompute(math.ceil(lum.shape[0] * lum.shape[1] / 128))
    illum = compute.GetData(3).reshape(lum.shape)

# Variables for light source detection & output
maxPeak = 0.0
mask = np.zeros((illum.shape[0] + 2, illum.shape[1] + 2), dtype=np.uint8)
rng = np.random.default_rng(0)
light_map = np.copy(pan_hdr)
lightsJson = []
lights = []

# 5.2) Find directional light source, based on pixels without depth

# Create compute shader
with ShaderManager(".\\HDRLib\\Shader\\SunReproject.comp") as compute:
    # Arbitrary, but small panoramic size
    imgSize = (100, 50)
    # Create output buffers (updated every frame)
    outputRGB = np.zeros((imgSize[1], imgSize[0], 4), dtype=np.float32)
    outputIllum = np.zeros((imgSize[1], imgSize[0], 2), dtype=np.float32)
    # For each frame (masked)
    for frame, vals in frames.items():
        # Load current frame values
        _, _, rgbmask, _, extr, _ = vals
        # Invert the camera matrix
        extrInv = np.linalg.inv(extr)
        # Load exposure
        ex = np.array(exposure[int("".join(filter(str.isdigit, frame)))], dtype=np.float32)
        # Masked RGB: uvec4
        compute.SetData(1, np.int32(rgbmask))
        # Vars: mat4, vec2, vec2, vec2, vec2, float
        camVars = np.concatenate((np.float32(extrInv.transpose().flatten()),
            np.array(intr_o, dtype=np.float32), np.float32(imgSize), ex))
        compute.SetData(2, camVars)
        # Output rgb buffer: vec4
        compute.SetData(3, outputRGB)
        # Output illum buffer: vec4
        compute.SetData(4, outputIllum)
        # Run compute shader
        compute.StartCompute(math.ceil((rgbmask.shape[1] * rgbmask.shape[0]) / 128))
        # Load & update the result
        outputRGB = compute.GetData(3).reshape(imgSize[1], imgSize[0], 4)
        outputIllum = compute.GetData(4).reshape(imgSize[1], imgSize[0], 2)

    # Calculate the average rgb & illumination (multiple samples / pixel)
    dirRGB = outputRGB[:,:,:3] / outputRGB[:,:,3:4]
    dirIllum = outputIllum[:,:,:1] / outputIllum[:,:,1:2]
    # Find brightest spot & corresponding light source
    dirMask = np.zeros((dirIllum.shape[0] + 2, dirIllum.shape[1] + 2), dtype=np.uint8)
    dirFiltered, dirPeak, dirPeakIdx = find_brightest_spot(dirIllum, dirMask[1:-1, 1:-1], 3)
    dirMask, _, dirParams = find_light_source(dirFiltered, dirMask, dirPeakIdx, 0.3, dirRGB)
    # The first (brightest) light source is the main directional light
    dirPeakRGB = np.average(dirRGB[dirPeakIdx[1], dirPeakIdx[0]])
    dirLng, dirLat = (dirPeakIdx[0] / imgSize[1]) * math.pi, (dirPeakIdx[1] / imgSize[0]) * 2.0 * math.pi
    dirShouldAdd, _, dirLightJson = build_directional_light(dirRGB, dirLat, dirLng, dirPeakRGB, dirParams)
    # If light is bright enough to be relevant
    if dirShouldAdd:
        # Append this light as json to output file
        lightsJson.append(dirLightJson)

# 5.3) Detect point light sources, based on reprojected radiance

# Number of point lights is unknown
while True:
    # Find the brightest spot
    filtered, peak, peakIdx = find_brightest_spot(illum, mask[1:-1, 1:-1], 21)
    # Find the corresponding light source & parameters
    mask, lightEllipse, lightParams = find_light_source(filtered, mask, peakIdx, 0.4, pan_hdr)
    # Update peak
    maxPeak = (maxPeak, peak)[peak > maxPeak]
    # Stop if not at least as bright as 80% of the peak
    if peak < 0.8  * maxPeak:
        break
    else:
        # Mark light source in panorama
        light_map = cv2.ellipse(light_map, lightEllipse, rng.random(3).tolist(), -1)
        # Load peak rgb value
        peakRGB = np.average(pan_hdr[peakIdx[1], peakIdx[0]])
        # Optimize light & build for output
        shouldAdd, light, lightJson = build_point_light(pan_hdr, pan_depth, best_candidate, peakRGB, lightParams)
        # If light is bright enough to be relevant
        if shouldAdd:
            # Append light as json to output file
            lightsJson.append(lightJson)
            # For rendering
            lights.append(light)

# Store light source information as json
with open(os.path.join(temp_path, os.pardir, "lights.json"), mode="w") as stream:
    json.dump(lightsJson, stream, indent=1)

# 5.4) Render detected point light sources (as spherical gaussians)

# Create compute shader
with ShaderManager(".\\HDRLib\\Shader\\SGRender.comp") as compute:
    imgSize = (pan_depth.shape[1], pan_depth.shape[0])
    # Build input data
    vars = np.concatenate((
        np.array(imgSize, dtype=np.int32).view("float32"),
        np.array([len(lights), 0.0], dtype=np.int32).view("float32"),
        np.array(lights, dtype=np.float32).flatten()
    ))
    # Set input data: uvec2, int, int, [vec4, vec4, vec4]
    compute.SetData(1, vars)
    results = np.zeros((imgSize[0], imgSize[1], 4), dtype=np.float32)
    compute.SetData(2, results.flatten())
    # Compute spherical gaussian lighting with predicted lights
    compute.StartCompute(math.ceil((imgSize[0] * imgSize[1]) / 128))
    results = compute.GetData(2).reshape((imgSize[0], imgSize[1], 4))
    sg_render = results[:,:,:3].reshape((imgSize[1], imgSize[0], 3))

# Possibly store debug info
if store_debug_panorama:
    cv2.imwrite(os.path.join(temp_path, os.pardir, "panorama_hdr.hdr"), pan_hdr)
    cv2.imwrite(os.path.join(temp_path, os.pardir, "panorama_depth.hdr"), pan_depth)
    cv2.imwrite(os.path.join(temp_path, os.pardir, "panorama_lum.hdr"), illum)
    cv2.imwrite(os.path.join(temp_path, os.pardir, "panorama_lights.hdr"), light_map)
    cv2.imwrite(os.path.join(temp_path, os.pardir, "panorama_sg.hdr"), sg_render)

# Done raytracing
rt.close()

# Cleanup
rmtree(temp_path)
