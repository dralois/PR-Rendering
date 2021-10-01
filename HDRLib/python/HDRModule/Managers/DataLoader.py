from ..Utils.ImgProcUtils import *
from ..Utils.MeshUtils import *

import numpy as np
np.seterr(invalid='ignore')

import cv2
import os

def load_path(path, correct_gamma):
    # Output data
    frames = {}

    # Scan the given path
    with os.scandir(path) as dir:
        for direntry in dir:
            # Frames dir?
            if direntry.is_dir() and direntry.name == "rgbd":
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
                    rgb = org if not correct_gamma else adjust_gamma(org, 2.2)
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

    # Return loaded data
    return frames, intr_c, intr_o, mesh
