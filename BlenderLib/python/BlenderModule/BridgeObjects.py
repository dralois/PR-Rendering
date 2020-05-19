# TODO
# Improve light & etc

class CXXMesh(object):
    def __init__(self):
        self.Name = ""
        self.File = ""
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)
        self.Shader = ""

class CXXLight(object):
    def __init__(self):
        self.Intensity = 1.0
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)

class CXXCamera(object):
    def __init__(self):
        self.Position = (0.0,0.0,0.0)
        self.Rotation = (0.0,0.0,0.0,0.0)
        self.Scale = (1.0,1.0,1.0)
        self.FOV = (1.0,1.0)
        self.Shift = (0.0,0.0)
        self.Output = ""
