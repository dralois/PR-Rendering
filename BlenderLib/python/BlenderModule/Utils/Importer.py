import sys

# Singleton storage
__orgPaths = None
__bpyPaths = None

# Returns bpy import
def ImportBpy():
    functional, broken = GetPaths()
    sys.path = functional
    import bpy
    sys.path = broken
    return bpy

# Returns [functional sys.path, broken sys.path with bpy]
def GetPaths() -> (str, str):
    global __orgPaths, __bpyPaths
    # Store the paths before and after breaking it
    if __orgPaths is None and __bpyPaths is None:
        __orgPaths = list(sys.path)
        import bpy
        __bpyPaths = list(sys.path)
    # Return them
    return __orgPaths, __bpyPaths

# Sets [functional sys.path, broken sys.path with bpy]
def SetPaths(orgPaths, bpyPaths):
    global __orgPaths, __bpyPaths
    # Store the paths before and after breaking it
    __orgPaths = orgPaths
    __bpyPaths = bpyPaths
