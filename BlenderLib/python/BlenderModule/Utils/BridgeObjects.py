# Import from C++ here

# FIXME: Shader params
# Mesh proxy object
class CXXMesh(object):
    def __init__(self):
        self.Name = ""
        self.File = ""
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)
        self.Shader = ""

# FIXME: Light specific params
# Light proxy object
class CXXLight(object):
    def __init__(self):
        self.Name = ""
        self.Type = "POINT"
        self.Color = (1.0,1.0,1.0)
        self.Intensity = 1.0
        self.Exposure = 1.0
        self.CastsIndirect = True
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)

# Camera proxy object
class CXXCamera(object):
    def __init__(self):
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)
        self.FOV = (0.6911,0.4711)
        self.Shift = (0.0,0.0)
        self.Result = ""

# Render settings
class CXXSettings(object):
    def __init__(self):
        self.Resolution = (1080,1920)
        self.DepthOnly = False
        self.Plugin = "blenderseed.zip"
        self.Output = "output//"
