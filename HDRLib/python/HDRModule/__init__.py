__all__ = ["Utils", "Managers"]

from .Managers.Estimator import EstimationManager

import sys
import os

# Called from C++
def SetupMultiprocessing():
    from platform import system
    from multiprocessing import set_executable, set_start_method
    # Set argv (not set for embedded interpreters)
    sys.argv = [FullPath(f"{FileDir(__file__)}/../__init__.py")]
    # Determine executable path
    if system() == "Windows":
        exePath = f"{os.path.split(FileDir(os.__file__))[0]}/python.exe"
    else:
        exePath = "/usr/bin/python3.7"
    # Set executable path & start method
    set_start_method("spawn", True)
    set_executable(FullPath(exePath))

# Full path from (relative) path
def FullPath(filePath):
    return os.path.realpath(filePath)

# Full directory from (relative) path / file
def FileDir(filePath):
    if os.path.isdir(FullPath(filePath)):
        return filePath
    else:
        return os.path.split(FullPath(filePath))[0]
