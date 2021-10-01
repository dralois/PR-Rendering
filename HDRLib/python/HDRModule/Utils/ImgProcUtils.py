import numpy as np
import cv2

import math

def adjust_gamma(image, gamma=1.0):
    invGamma = 1.0 / gamma
    table = np.array([((i / 255.0) ** invGamma) * 255
        for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(image, table)

def gradient_map(image, maxGrad=200.0):
    gray = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
    dx = cv2.Sobel(gray, cv2.CV_32F, 1, 0, ksize=3)
    dy = cv2.Sobel(gray, cv2.CV_32F, 0, 1, ksize=3)
    combined = np.maximum(np.absolute(dx), np.absolute(dy))
    return np.greater(combined, maxGrad)

def load_extr(file) -> np.ndarray:
    return np.loadtxt(file, dtype="f", delimiter=" ")

def load_intr(string) -> np.ndarray:
    arr = np.fromstring(string, dtype = "f", sep = " ")
    return np.reshape(arr, (4, 4))

# See https://en.wikipedia.org/wiki/Relative_luminance
def raw_luminance(rgb):
    return (rgb[:,:,0] * 0.2126 + rgb[:,:,1] * 0.7152 + rgb[:,:,2] * 0.0722) * 179.0

# See https://en.wikipedia.org/wiki/Exposure_value#EV_as_a_measure_of_luminance_and_illuminance
def illuminance_to_ev(il):
    return math.log2(il / 2.5)

# See https://en.wikipedia.org/wiki/Exposure_value#EV_as_a_measure_of_luminance_and_illuminance
def ev_to_illuminance(ev):
    return 2.5 * math.pow(ev, 2.0)

# See https://www.pyimagesearch.com/2014/09/29/finding-brightest-spot-image-using-python-opencv/
def find_brightest_spot(image, mask, radius):
    filtered = cv2.GaussianBlur(image, (radius, radius), 0)
    _, peak, _, peakIdx = cv2.minMaxLoc(filtered, 255 - mask)
    return filtered, peak, peakIdx

# See https://stackoverflow.com/questions/16705721/opencv-floodfill-with-mask
def find_light_source(illum, mask, peakIdx, diff, rgb):
    # 8 neighbor radius & compare to seed
    flags = 8
    flags |= cv2.FLOODFILL_MASK_ONLY
    flags |= cv2.FLOODFILL_FIXED_RANGE
    flags |= (255 << 8)

    # Find light area using flood fill
    updatedMask = np.copy(mask)
    peakDiff = illum[peakIdx[1], peakIdx[0]] * diff
    cv2.floodFill(illum, updatedMask, peakIdx, 255, peakDiff, peakDiff, flags)

    # Calculate the lights illumination intensity
    lightMask = np.where(updatedMask[1:-1, 1:-1] - mask[1:-1, 1:-1], False, True)
    lightIntensity = np.sum(np.ma.array(illum, mask=lightMask))
    lightEV = illuminance_to_ev(lightIntensity)

    # Find corresponding pixels and fit ellipse
    lightPoints = np.roll(np.stack(np.nonzero(updatedMask[1:-1, 1:-1] - mask[1:-1, 1:-1]), axis=1), 1, axis=1)
    lightEllipse = cv2.fitEllipse(lightPoints)

    # Calculate solid angle of light
    lightSolidAngle = ellipse_solid_angle(lightEllipse, rgb.shape)
    # Calculate mean light color
    colMask = lightMask.repeat(3, axis=1).reshape(lightMask.shape[0], -1, 3)
    lightColor = np.mean(np.ma.array(rgb, mask=colMask)[lightPoints[:,1], lightPoints[:,0]], axis=0) / lightEV

    # Return mask with new light, fitted ellipse & light params
    return updatedMask, lightEllipse, (lightPoints, lightColor, lightEV, lightSolidAngle, lightMask)

def build_point_light(rgb, depth, camPos, peak, lightParams):
    from .SolverUtils import solve_intensity

    # Extract params
    lightPoints, lightColor, lightEV, lightSolidAngle, lightMask = lightParams

    # Backproject to find its position
    lightPos = pan_depth_to_point(depth, lightPoints, camPos)

    # Build light array (for minimization & later rendering)
    light = np.concatenate((
                np.append(lightColor * lightEV, 0.0).data,
                np.append((lightPos - camPos) / np.linalg.norm(lightPos - camPos), 0.0),
                np.array([4.0 * math.pi / lightSolidAngle, 0.0, 0.0, 0.0], dtype=np.float32)
            ))

    # Build (current light) mask for minimization
    minMask = ~lightMask.repeat(2, axis=1).reshape(lightMask.shape[0],-1,2)

    # Perform minimization to get more accurate intensity (exposure)
    evSolve = np.average(solve_intensity(rgb, minMask, light, peak) / lightColor)

    # Build json entry for light
    lightJson = {
        "type" : "POINT",
        "position" : lightPos.tolist(),
        "color" : lightColor.tolist(),
        "exposure" : evSolve
    }

    # Return light & json entry
    return evSolve > 0.0, light, lightJson

def build_directional_light(rgb, lat, lng, peak, lightParams):
    from .SolverUtils import solve_intensity
    from .OutputMuter import StdMute

    # Numba warning is irrelevant
    with StdMute():
        import quaternion

    # Axis vector for minimization needs to be rotated
    n = np.array([-math.sin(lng) * math.sin(lat), math.cos(lng) * math.sin(lat), math.cos(lat)], dtype=np.float32)
    # Forward vector is vector towards the found peak
    f = np.array([-math.cos(lng) * math.sin(lat), math.cos(lat), -math.sin(lat) * math.sin(lng)], dtype=np.float32)
    # Calculate up and right accordingly (as long as it forms a base it will work)
    r = np.cross(f, np.array([0.0, 0.0, 1.0]))
    u = np.cross(r, f)
    # Calculate the quaternion from the rotation matrix
    # See https://www.3dgep.com/understanding-the-view-matrix/#look-at-camera
    dirQuat = quaternion.from_rotation_matrix(np.stack((r, u, -f), axis=1))

    # Extract light params
    _, lightColor, lightEV, lightSolidAngle, lightMask = lightParams

    # Build light array (for minimization & later rendering)
    light = np.concatenate((
                np.append(lightColor * lightEV, 0.0).data,
                np.append((n) / np.linalg.norm(n), 0.0),
                np.array([4.0 * math.pi / lightSolidAngle, 0.0, 0.0, 0.0], dtype=np.float32)
            ))

    # Build (current light) mask for minimization
    minMask = ~lightMask.repeat(2, axis=1).reshape(lightMask.shape[0],-1,2)

    # Perform minimization to get more accurate intensity (exposure)
    evSolve = np.average(solve_intensity(rgb, minMask, light, peak) / lightColor)

    # Build json entry for light
    lightJson = {
        "type" : "SUN",
        "rotation" : np.append(dirQuat.imag, dirQuat.real).tolist(),
        "color" : lightColor.tolist(),
        "exposure" : evSolve
    }

    # Return light & json entry
    return evSolve > 0.0, light, lightJson

def pixel_to_vec(pixel, size):
    # Calculate longitude & latitude
    lng = ((pixel[0] + 0.5) / size[1]) * 2 * math.pi
    lat = (1.0 - ((pixel[1] + 0.5) / size[0])) * math.pi
    # Normalized corresponding vector
    return np.array([-math.sin(lng) * math.sin(lat), math.cos(lng) * math.sin(lat), -math.cos(lat)], dtype=np.float32)

def pan_depth_to_point(depthMap, lightPoints, camPos):
    # Calculate center of mass of light source pixels
    pixel = np.uint32(np.mean(lightPoints, axis=0))
    # Calculate average depth
    depth = np.mean(depthMap[lightPoints[:,1], lightPoints[:,0]])
    # Calculate vector to pixel on sphere
    vec = pixel_to_vec(pixel, depthMap.shape)
    # Return light position (camera + direction * distance)
    return camPos + (vec * depth)

# See https://en.wikipedia.org/wiki/Solid_angle#Cone,_spherical_cap,_hemisphere
def ellipse_solid_angle(ellipse, size):
    ellipse_pos = np.append(ellipse[0], 0)
    ellipse_axes = ellipse[1]
    ellipse_angle = ellipse[2]

    # Build rotation matrix
    rotMat = cv2.getRotationMatrix2D(ellipse[0], ellipse_angle, 1.0)
    # Calculate extrema points on unit sphere
    p0_0 = pixel_to_vec(np.matmul(rotMat, ellipse_pos + np.array([ellipse_axes[0], 0, 0])), size)
    p0_1 = pixel_to_vec(np.matmul(rotMat, ellipse_pos - np.array([ellipse_axes[0], 0, 0])), size)
    p1_0 = pixel_to_vec(np.matmul(rotMat, ellipse_pos + np.array([0, ellipse_axes[1], 0])), size)
    p1_1 = pixel_to_vec(np.matmul(rotMat, ellipse_pos - np.array([0, ellipse_axes[1], 0])), size)
    # Calculate angle between extrema points
    a0 = math.acos(np.dot(p0_0, p0_1))
    a1 = math.acos(np.dot(p1_0, p1_1))

    # Calculate solid angle of fitted cone (only works if cone apex angle < pi!)
    return 2.0 * math.pi * (1.0 - math.cos((a0 + a1) / 4.0))
