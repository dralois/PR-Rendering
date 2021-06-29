from .Utils.ImgProcUtils import *
from .Utils.MeshUtils import *
from .Utils.SolverUtils import *

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
curr_path = ""

# TODO: For each scene
with os.scandir(test_path) as dir:
    for direntry in dir:
        # Frames dir?
        if direntry.is_dir() and direntry.name == frames_folder:
            # Required to store intermediate results
            curr_path = os.path.join(direntry.path, os.pardir)
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
                rgb = cv2.cvtColor(
                        adjust_gamma(
                            cv2.imread(os.path.join(direntry.path, base + ".color.jpg"), cv2.IMREAD_COLOR),
                        2.2),
                    cv2.COLOR_BGR2RGB)
                depth = cv2.imread(os.path.join(direntry.path, base + ".depth.pgm"), cv2.IMREAD_ANYDEPTH)
                extr = load_extr(os.path.join(direntry.path, base + ".pose.txt"))
                grads = gradient_map(rgb)
                # Extract camera position
                eye = extr[0:3, 3:4]
                # Convert camera2world -> world2camera
                flip = np.array([[ 1,  0,  0,  0],
                                [ 0, -1,  0,  0],
                                [ 0,  0, -1,  0],
                                [ 0,  0,  0,  1]])
                extr = np.linalg.inv(np.matmul(extr, flip))
                # Store
                frames[base] = (rgb, depth, grads, extr, eye)
        # Load mesh
        elif direntry.is_file() and direntry.name == "mesh.refined.v2.obj":
            mesh = load_mesh(direntry.path, "obj")

# 2) Project onto scene geometry
# 2.1) Raytrace all vertices -> all cameras, locate pixel if not occluded

size = math.ceil(math.sqrt(len(mesh.vertices)))
vertexPixels = {fr : [None for _ in range(len(mesh.vertices))] for fr in frames}

# Necessary to only process one frame at a time
renderDone = threading.Event()

# Locates hit pixels
def build_hit_image(rt: NpOptiX) -> None:
    # Load hit data (fid: hit face indices, pid: hit vertex index)
    fid = rt._geo_id[:,:,1].reshape(rt._width, rt._height)
    pid = rt._geo_id[:,:,0].reshape(rt._width, rt._height)
    pid &= 0xC0000000
    pid >>= 30

    for idx, x in np.ndenumerate(fid):
        # If face was hit (-> has index)
        if x < 0xFFFFFFFF:
            # Calculate vertex index
            hitFace = mesh.faces[x]
            hitVertex = hitFace[pid[idx]]
            # If original vertex matches hit position
            if np.allclose(mesh.vertices[hitVertex], data[idx][:3]):
                # Calculate camera space position
                campos = np.matmul(extr, np.append(data[idx][:3], 1))[:3]
                # In front of the camera?
                if campos[2] < 0.0:
                    # Calculate uv coords
                    uvs = uv_coords(campos, intr)
                    # Within camera frame?
                    if min(uvs) >= 0 and max(uvs) <= 1.0:
                        # Vertex hit, compute pixel
                        px = pixels(uvs, intr)
                        # Pixel not on strong gradient?
                        if not grads[px]:
                            # Hit, load & store pixel
                            col = rgb[px]
                            vertexPixels[frame][hitVertex] = col
                            # Potentially set vertex color in mesh
                            if store_debug_mesh:
                                set_vertex_color(mesh, hitVertex, col)
    # Next frame can be processed
    renderDone.set()

# Load farmework
rt = NpOptiX(on_rt_accum_done=build_hit_image, width=size, height=size, start_now=False)
rt.set_param(min_accumulation_step=1, max_accumulation_frames=1)

# Build ray targets texture
data = np.full((size*size, 4), -1.0, dtype=np.float32)
data[:mesh.vertices.shape[0],:3] = mesh.vertices[:,:3]
data = data.reshape(size, size, 4)
rt.set_texture_2d("target", data)

# Create camera & mesh
rt.setup_material("flat", m_flat)
rt.setup_camera("cam", cam_type="CustomProjXYZ", textures=["target"])
rt.set_mesh("mesh", mesh.vertices, mesh.faces, mat="flat")

# Start raytracing
rt.start()

# Try loading intermediate results
try:
    with open(os.path.join(curr_path, "RTResults.pkl"), mode="r+b") as stream:
        vertexPixels = pickle.load(stream)
except OSError:
    # Raytrace all frames
    for frame, vals in frames.items():
        # Load current frame values & update camera position
        rgb, _, grads, extr, eye_pos = vals
        rt.update_camera("cam", eye=eye_pos)
        rt.refresh_scene()

        # One frame at a time
        renderDone.wait()
        renderDone.clear()

        # Potentially store colored mesh
        if store_debug_mesh:
            store_mesh(f"{test_path}\\{frames_folder}\\{frame}_mesh.glb", mesh)

    # Store intermediate results
    with open(os.path.join(curr_path, "RTResults.pkl"), mode="x+b") as stream:
        pickle.dump(vertexPixels, stream)

# Select random subset of max 200.000 verts
vert_idx = random.sample(range(min(len(mesh.vertices), solve_max_verts)), min(len(mesh.vertices), solve_max_verts))
selection = np.array([np.take(pixels, vert_idx) for _, pixels in vertexPixels.items()], dtype=object).transpose()

# Solve for exposure
exposure = SolveExposure(selection)

# Done raytracing
rt.close()