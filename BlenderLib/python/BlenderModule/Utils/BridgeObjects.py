# Import from C++ here

# TODO
class CXXMesh(object):
    def __init__(self):
        self.Name = ""
        self.File = ""
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)
        self.Shader = ""

# TODO
class CXXLight(object):
    def __init__(self):
        self.Intensity = 1.0
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)

# TODO
class CXXCamera(object):
    def __init__(self):
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)
        self.FOV = (1.0,1.0)
        self.Shift = (0.0,0.0)
        self.Result = ""

# TODO
class CXXSettings(object):
    def __init__(self):
        self.Resolution = (1080,1920)
        self.DepthOnly = False
        self.Plugin = "blenderseed.zip"
        self.Output = "output//"
