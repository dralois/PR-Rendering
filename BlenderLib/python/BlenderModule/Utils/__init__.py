__all__ = ["Importer", "Logger", "ShaderCompiler", "TextureConverter"]

import os
import sys

# Called from C++
def SetupMultiprocessing():
    # Set argv (not set for embedded interpreters)
    sys.argv = [FullPath(f"{FileDir(__file__)}\\..\\__init__.py")]
    # Deduce executable path
    from platform import system
    exeFile = "python.exe" if system() == "Windows" else "bin\\python"
    exePath = f"{os.path.split(FileDir(os.__file__))[0]}\\{exeFile}"
    # Set executable path & start method
    from multiprocessing import set_executable, set_start_method
    set_executable(FullPath(exePath))
    set_start_method("spawn")

# Full path from (relative) path
def FullPath(filePath):
    return os.path.realpath(filePath)

# Full directory from (relative) path / file
def FileDir(filePath):
    if os.path.isdir(FullPath(filePath)):
        return filePath
    else:
        return os.path.split(FullPath(filePath))[0]

# Full file name from path
def FullFileName(filePath):
    if os.path.isdir(FullPath(filePath)):
        raise ValueError
    else:
        return os.path.split(FullPath(filePath))[1]

# Get file name from path
def FileName(filePath):
    return os.path.splitext(FullFileName(filePath))[0]

# Get file extension from path
def FileExt(filePath):
    return os.path.splitext(FullFileName(filePath))[1]

# Decorator for class properties
class classproperty(object):
    def __init__(self, f):
        self.f = f
    def __get__(self, obj, owner):
        return self.f(owner)
