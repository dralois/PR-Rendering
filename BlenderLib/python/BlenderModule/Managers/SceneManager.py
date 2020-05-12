import bpy

from ..Converters import Camera, Mesh, Lights

class SceneObjects(object):

    def __init__(self):
        self.__cameras : list[Camera.CameraConverter] = []
        self.__lights : list[Lights.LightConverter] = []
        self.__meshes : list[Mesh.MeshConverter] = []

    def __del__(self):
        del self.__cameras
        del self.__lights
        del self.__meshes

    # Create and return new camera
    def AddCamera(self, name):
        newCam = Camera.CameraConverter(name)
        self.__cameras.append(newCam)
        return newCam

    # Create and return new mesh
    def AddMesh(self, name):
        newMesh = Mesh.MeshConverter(name)
        self.__meshes.append(newMesh)
        return newMesh

    # Create and return new light
    def AddLight(self, name, type = "POINT"):
        newLight = None
        # Create respective light
        if type == "SPOT":
            newLight = Lights.SpotLightConverter(name)
        elif type == "SUN":
            newLight = Lights.SunLightConverter(name)
        elif type == "AREA":
            newLight = Lights.AreaLightConverter(name)
        else:
            newLight = Lights.PointLightConverter(name)
        # Add and return it
        self.__lights.append(newLight)
        return newLight

    # Try to render and remove next camera
    def TryRenderNextCamera(self):
        # If any cameras left
        if len(self.__cameras) > 0:
            # Render and remove it
            nextRender = self.__cameras.pop()
            nextRender.ActivateAndRender()
            del nextRender
            # Success
            return True
        else:
            # Stop rendering
            return False
