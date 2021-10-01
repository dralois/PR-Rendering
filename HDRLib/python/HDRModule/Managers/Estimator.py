from .DataLoader import *
from .Raytracer import *
from .LightDetector import *

import multiprocessing as mp
import numpy as np
np.seterr(invalid='ignore')

import cv2
import os
import json

gamma_correct = True
store_debug_mesh = False
store_debug_panorama = False
randomize_samples = True
solve_max_verts = 200000

class EstimationManager(object):

    def __init__(self):
        self.__process = mp.Process()

    def close(self):
        try:
            if self.__process.is_alive():
                self.__process.terminate()
        except ValueError:
            pass

    def estimate(self, path):
        # Create new estimator process
        self.__process = mp.Process(target=_estimate_process, args=(path,), daemon=True)
        # No reuse, wait for termination and close
        self.__process.start()
        self.__process.join()
        self.__process.close()

def _estimate_process(path):
    # Init raytracing
    rt = Raytracer()

    # 1) Import all images, inverse gamma correct them (y = 2.2)
    frames, intr_c, intr_o, mesh = load_path(path, True)

    # 2) Project onto scene geometry

    # 2.1) Get vertices / radiance samples
    selection = rt.vertex_hitmap(mesh, frames, intr_c, solve_max_verts, randomize_samples)

    # 2.2) Solve for exposure
    exposure = solve_exposure(selection)

    # Store solved exposure (convert to EV -> logarithmic) for future use
    with open(os.path.join(path, "exposures.json"), mode="w") as stream:
        exposures = dict(zip(frames.keys(), np.log2(exposure)[:,0].tolist()))
        json.dump(exposures, stream, indent=1)

    # 3) Reproject radiance
    samples = rt.radiance_reproject(mesh, frames, intr_c, exposure)

    # Potentially store "HDR" mesh
    if store_debug_mesh:
        # Store & output reprojected colors
        set_vertex_colors(mesh, samples * 255.0)
        store_mesh(os.path.join(path, "hdr_mesh.glb"), mesh)

    # 4) Render HDR panorama
    pan_hdr, pan_depth, best_candidate = rt.render_panorama(mesh, frames, samples)

    # 5) Detect light sources

    # 5.1) Compute illuminance
    illum = luminance_compute(pan_hdr)

    # 5.2) Find directional light source, based on pixels without depth
    dirLightsJson = detect_sun(frames, exposure, intr_o)

    # 5.3) Detect point light sources, based on reprojected radiance
    lightsJson, lights, light_map = detect_lights(pan_hdr, pan_depth, illum, best_candidate)
    lightsJson.extend(dirLightsJson)

    # Store light source information as json
    with open(os.path.join(path, "lights.json"), mode="w") as stream:
        json.dump(lightsJson, stream, indent=1)

    # Possibly store debug info
    if store_debug_panorama:
        cv2.imwrite(os.path.join(path, "panorama_hdr.hdr"), pan_hdr)
        cv2.imwrite(os.path.join(path, "panorama_depth.hdr"), pan_depth)
        cv2.imwrite(os.path.join(path, "panorama_lum.hdr"), illum)
        cv2.imwrite(os.path.join(path, "panorama_lights.hdr"), light_map)
        # Render detected point light sources (as spherical gaussians)
        sg_render = debug_detected_lights(pan_depth, lights)
        cv2.imwrite(os.path.join(path, "panorama_sg.hdr"), sg_render)

    # End raytracing
    rt.close()
