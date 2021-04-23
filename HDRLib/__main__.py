from .Utils.ImgProcUtils import adjust_gamma, load_intr, load_extr
from .Utils.MeshUtils import load_mesh
import cv2
import os

# 1) Import all images, inverse gamma correct them (y = 2.2)

frames = {}
mesh = None
intr = None

# TODO: For each scene
with os.scandir(".\\HDRLib\\Test\\1") as dir:
    for direntry in dir:
        # Frames dir?
        if direntry.is_dir() and direntry.name == "rgbd":
            # Load info file
            info = {}
            for line in open(os.path.join(direntry.path, "_info.txt"), "r").readlines():
                direntry = line.split("=")
                info[direntry[0].strip()] = direntry[1].strip()
            # Load camera intrinsics
            intr = load_intr(info["m_calibrationColorIntrinsic"])[0:3, 0:3]
            # Loop frames
            for i in range(0, int((len(os.listdir(direntry.path)) - 1) / 3)):
                # Read frame, depth, pose
                base = "frame-{0:06d}".format(i)
                img = adjust_gamma(cv2.imread(os.path.join(direntry.path, base + ".color.jpg"), cv2.IMREAD_COLOR), 2.2)
                depth = cv2.imread(os.path.join(direntry.path, base + ".depth.pgm"), cv2.IMREAD_ANYDEPTH)
                extr = load_extr(os.path.join(direntry.path, base + ".pose.txt"))
                # Convert camera2world -> world2camera
                extract_r = extr[0:3, 0:3].transpose()
                extract_t = -1.0 * extr[0:3, 3:4]
                # Store
                frames[base] = (img, depth, extract_r, extract_t)
        # Load mesh
        elif direntry.is_file() and direntry.name == "mesh.refined.v2.obj":
            mesh = load_mesh(direntry.path, "obj")

# 2) Project onto scene geometry
# 2.1) Raytrace all vertices -> all cameras, locate pixel if not occluded
