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

def raw_luminance(img):
    return (img[:,:,0] * 0.2126 + img[:,:,1] * 0.7152 + img[:,:,2] * 0.0722) * 179.0

def illuminance_to_ev(il):
    import math
    return math.log2(il / 2.5)
