__all__ = ["Importer", "Logger", "ShaderCompiler"]

import os

# Full path from (relative) path
def FullPath(filePath):
    return os.path.abspath(filePath)

# Full directory from (relative) path / file
def FileDir(filePath):
    if os.path.isdir(FullPath(filePath)):
        return path
    else:
        return os.path.split(FullPath(filePath))[0] + "\\"

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
