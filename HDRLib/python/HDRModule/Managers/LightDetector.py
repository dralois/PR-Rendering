from ..Utils.ImgProcUtils import *
from ..Utils.SolverUtils import *
from ..Utils.ShaderUtils import *

import numpy as np
np.seterr(invalid='ignore')

import math

def luminance_compute(pan_hdr):
    # Calculate raw luminance
    lum = raw_luminance(pan_hdr)

    # Create compute shader
    with ShaderManager("Luminance.comp") as compute:
        # Set data
        compute.SetData(1, lum.flatten())
        compute.SetData(2, np.array([lum.shape[1], lum.shape[0]]))
        output = np.zeros_like(lum, dtype=np.float32)
        compute.SetData(3, output.flatten())
        # Compute illuminance
        compute.StartCompute(math.ceil(lum.shape[0] * lum.shape[1] / 128))
        illum = compute.GetData(3).reshape(lum.shape)

    # Return illuminance
    return illum

def detect_sun(frames, exposure, intr_o):
    # Output json
    lightsJson = []

    # Create compute shader
    with ShaderManager("SunReproject.comp") as compute:
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
        dirFiltered, _, dirPeakIdx = find_brightest_spot(dirIllum, dirMask[1:-1, 1:-1], 3)
        dirSuccess, dirMask, _, dirParams = find_light_source(dirFiltered, dirMask, dirPeakIdx, 0.3, dirRGB)
        # If a light was detected
        if dirSuccess:
            # The first (brightest) light source is the main directional light
            dirPeakRGB = np.average(dirRGB[dirPeakIdx[1], dirPeakIdx[0]])
            dirLng, dirLat = (dirPeakIdx[0] / imgSize[1]) * math.pi, (dirPeakIdx[1] / imgSize[0]) * 2.0 * math.pi
            dirShouldAdd, _, dirLightJson = build_directional_light(dirRGB, dirLat, dirLng, dirPeakRGB, dirParams)
            # If light is bright enough to be relevant
            if dirShouldAdd:
                # Append this light as json to output file
                lightsJson.append(dirLightJson)

    # Return detected light as json
    return lightsJson

def detect_lights(pan_hdr, pan_depth, illum, best_candidate):
    # Create resources
    mask = np.zeros((illum.shape[0] + 2, illum.shape[1] + 2), dtype=np.uint8)
    rng = np.random.default_rng(0)
    light_map = np.copy(pan_hdr)
    maxPeak = 0.0

    # Output structures
    lightsJson = []
    lights = []

    # Number of point lights is unknown
    while True:
        # Find the brightest spot
        filtered, peak, peakIdx = find_brightest_spot(illum, mask[1:-1, 1:-1], 21)
        # Find the corresponding light source & parameters
        success, mask, lightEllipse, lightParams = find_light_source(filtered, mask, peakIdx, 0.4, pan_hdr)
        # Update peak
        maxPeak = (maxPeak, peak)[peak > maxPeak]
        # Stop if not at least as bright as 80% of the peak
        if peak < 0.8  * maxPeak:
            break
        elif not success:
            continue
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

    # Return lights as json and for debugging
    return lightsJson, lights, light_map

def debug_detected_lights(pan_depth, lights):
    # Create compute shader
    with ShaderManager("SGRender.comp") as compute:
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

    # Return rendered lights
    return sg_render
