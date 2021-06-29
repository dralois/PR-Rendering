import cv2
import numpy as np

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

def uv_coords(p, intr):
    return (0.5 - ((p[0] * intr[0]) / (p[2] * intr[4])) - intr[2],
            0.5 + ((p[1] * intr[1]) / (p[2] * intr[5])) - intr[3])

def pixels(uv, intr):
    return (int(intr[5] * uv[1]), int((intr[4] * uv[0])))
