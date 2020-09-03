from .Logger import GetLogger
from . import FileName, FileDir, FullPath

import os
import sys

logger = GetLogger()

colorSpaces = ["linear", "sRGB", "Rec709"]
colorDepths = ["default", "uint8", "sint8", "uint16", "sint16", "half", "float"]

# Converts a given image & returns path to new texture
def ConvertTexture(imageFile, colorSpace, colorDepth, modulePath):
    # Don't convert if texture exists
    outPath = f"{FileDir(imageFile)}/{FileName(imageFile)}.tx"
    if os.path.exists(outPath):
        return outPath

    # Store original path
    tempPath = os.environ["PATH"]
    pyDir = FullPath(f"{modulePath}/appleseed/lib")

    # Modify sys path and import tools
    sys.path.append(pyDir)
    sys.path.append(modulePath)
    import utils.path_util as path_utils

    # Modify env path and import appleseed
    binDir = path_utils.get_appleseed_bin_dir_path()
    os.environ["PATH"] += os.pathsep + binDir
    from appleseed._appleseedpython3 import oiio_make_texture

    # Undo modifications
    os.environ["PATH"] = tempPath
    sys.path.remove(modulePath)
    sys.path.remove(pyDir)

    # Convert texture & return new file
    colorSpace = colorSpace if colorSpace in colorSpaces else "sRGB"
    colorDepth = colorDepth if colorDepth in colorDepths else "default"
    logger.info(f"Converting texture {imageFile} ({colorSpace}, {colorDepth})")
    oiio_make_texture(imageFile, outPath, colorSpace, colorDepth)
    return outPath
