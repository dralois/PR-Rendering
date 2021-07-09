from .Utils.ImgProcUtils import *
from .Utils.MeshUtils import *
from .Utils.SolverUtils import *
from .Utils.ShaderUtils import *

from plotoptix import NpOptiX
from plotoptix.materials import m_flat
import numpy as np

import cv2
import math
import os
import threading
import random
import pickle

# 1) Import all images, inverse gamma correct them (y = 2.2)

# RGB, depth, gradients, extrinsics matrix, eye pos
frames = {}
mesh = None
intr = (0, 0, 0, 0, 0, 0)
frames_folder = "rgbd"
store_debug_mesh = False
solve_max_verts = 200000
test_path = ".\\HDRLib\\Test\\1"
temp_path = ""

# TODO: For each scene
with os.scandir(test_path) as dir:
    for direntry in dir:
        # Frames dir?
        if direntry.is_dir() and direntry.name == frames_folder:
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
            cx = ((intr_loaded[0][2] - (w / 2)) / max(w, h))
            cy = ((intr_loaded[1][2] - (h / 2)) / max(w, h))
            intr = (fx, fy, cx, cy, w, h)
            # Loop frames
            for i in range(0, max(1, int("".join(filter(str.isdigit, os.listdir(direntry.path)[-2]))))):
                # Read frame, depth, pose
                base = "frame-{0:06d}".format(i)
                org = cv2.cvtColor(
                        cv2.imread(os.path.join(direntry.path, base + ".color.jpg"), cv2.IMREAD_COLOR),
                        cv2.COLOR_BGR2RGB)
                rgb = adjust_gamma(org, 2.2)
                depth = cv2.imread(os.path.join(direntry.path, base + ".depth.pgm"), cv2.IMREAD_ANYDEPTH)
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
                frames[base] = (org, rgb, depth, grads, extr, eye)
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
rt.setup_camera("cam", cam_type="CustomProjXYZ", textures=["target"])
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
            camVars = np.concatenate((np.float32(extr.transpose().flatten()), np.array(intr, dtype=np.float32)))
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

            # Potentially store colored mesh
            if store_debug_mesh:
                # Set computed colors
                for x in range(0, len(mesh.vertices)):
                    col = vertexPixels[frame][x]
                    if not np.any(np.isnan(col)):
                        set_vertex_color(mesh, x, col)
                # Store
                store_mesh(f"{test_path}\\{frames_folder}\\{frame}_mesh.glb", mesh)

            # Next frame
            renderDone.clear()

        # Store intermediate results
        with open(os.path.join(temp_path, "VertexPixels.pkl"), mode="x+b") as stream:
            pickle.dump(vertexPixels, stream)

# 2.2) Solve for exposure & radiance

# Select random subset of max 200.000 verts
vert_idx = random.sample(range(min(len(mesh.vertices), solve_max_verts)), min(len(mesh.vertices), solve_max_verts))
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
            org, rgb, _, _, extr, eye_pos = vals
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
                np.array(intr, dtype=np.float32), np.zeros((2), dtype=np.float32), eye, oj, ex))
            compute.SetData(4, camVars)

            # Image data: uvec4
            rgbData = np.uint32(np.dstack((rgb, np.zeros((rgb.shape[0], rgb.shape[1], 1), dtype=np.uint8)))).reshape(-1, 4)
            compute.SetData(5, rgbData)

            # Original image data: uvec4
            orgData = np.uint32(np.dstack((org, np.zeros((org.shape[0], org.shape[1], 1), dtype=np.uint8)))).reshape(-1, 4)
            compute.SetData(6, orgData)

            # Output buffer: vec4
            output = np.full((len(mesh.vertices), 4), -1, dtype=np.float32)
            compute.SetData(7, output)

            # Calculate in shader
            compute.StartCompute(math.ceil(hitData.shape[0] / 128))
            result = compute.GetData(7)[:,:3]
            vertexRadiance[frame] = np.where(result >= 0.0, result, result * np.nan)

            # Next frame
            renderDone.clear()

        # Store intermediate results
        with open(os.path.join(temp_path, "Reprojection.pkl"), mode="x+b") as stream:
            pickle.dump(vertexRadiance, stream)

# Generate median vertex color samples
samples = np.array([vertices for _, vertices in vertexRadiance.items()]).transpose((1, 0, 2))
samples = np.nanmedian(samples[:,:,:3], axis=1)
samples = np.where(np.isnan(samples), np.zeros_like(samples), samples / 255.0)

# Update raytracing mesh
rt.set_mesh("mesh", mesh.vertices, mesh.faces, mat="flat", c=samples)

# Potentially store "HDR" mesh
if store_debug_mesh:
    # Store & output reprojected colors
    set_vertex_colors(mesh, samples * 255.0)
    store_mesh(os.path.join(temp_path, os.pardir, "hdr_mesh.glb"), mesh)

# 4) TODO



# Done raytracing
rt.close()
