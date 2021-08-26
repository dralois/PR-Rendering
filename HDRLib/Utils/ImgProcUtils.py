import numpy as np

import cv2

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
def raw_luminance(img):
    return (img[:,:,0] * 0.2126 + img[:,:,1] * 0.7152 + img[:,:,2] * 0.0722) * 179.0

# See https://en.wikipedia.org/wiki/Exposure_value#EV_as_a_measure_of_luminance_and_illuminance
def illuminance_to_ev(il):
    from math import log2
    return log2(il / 2.5)

# See https://www.pyimagesearch.com/2014/09/29/finding-brightest-spot-image-using-python-opencv/
def find_brightest_spot(image, mask, radius):
    filtered = cv2.GaussianBlur(image, (radius, radius), 0)
    _, peak, _, peakIdx = cv2.minMaxLoc(filtered, 255 - mask)
    return filtered, peak, peakIdx

# See https://stackoverflow.com/questions/16705721/opencv-floodfill-with-mask
def find_light_source(image, mask, peakIdx):
    # 8 neighbor radius & compare to seed
    flags = 8
    flags |= cv2.FLOODFILL_MASK_ONLY
    flags |= cv2.FLOODFILL_FIXED_RANGE
    flags |= (255 << 8)

    # Find light area using flood fill
    lightMask = np.copy(mask)
    peakDiff = image[peakIdx[1], peakIdx[0]] * 0.4
    cv2.floodFill(image, lightMask, peakIdx, 255, peakDiff, peakDiff, flags)

    # Calculate the lights illumination intensity
    lightIntensity = np.sum(np.ma.array(image, mask=np.where(lightMask[1:-1, 1:-1] - mask[1:-1, 1:-1], False, True)))
    # Find corresponding pixels and fit ellipse
    lightPoints = np.roll(np.stack(np.nonzero(lightMask[1:-1, 1:-1] - mask[1:-1, 1:-1]), axis=1), 1, axis=1)
    ellipse = cv2.fitEllipse(lightPoints)

    # Return detected light area & pixels
    return lightMask, lightPoints, lightIntensity, ellipse

def pan_depth_to_point(depth, pixel, size, camPos):
    from math import sin, cos, pi
    # Calculate longitude & latitude
    lng = (pixel[0] / size[1]) * 2 * pi
    lat = (1.0 - (pixel[1] / size[0])) * pi
    # Normalized corresponding vector
    vec = np.array([-sin(lng) * sin(lat), cos(lng) * sin(lat), -cos(lat)], dtype=np.float32)
    # Return light position (camera + direction * distance)
    return camPos + (vec * depth)

