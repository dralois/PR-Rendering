import cv2
import numpy as np

def adjust_gamma(image, gamma=1.0):
    invGamma = 1.0 / gamma
    table = np.array([((i / 255.0) ** invGamma) * 255
        for i in np.arange(0, 256)]).astype("uint8")
    return cv2.LUT(image, table)

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
