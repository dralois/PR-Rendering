# Define package includes
__all__ = ["CameraManager", "MaterialManager"]

import bpy

class RenderManager(object):

    def __init__(self):
        self.__renderer = None
    
    def __del__(self):
        try:
            self.__stop_rendering()
        except:
            pass

    def render(self):
        """
        Converts the provided scene and starts rendering
        """
        return

    def __stop_rendering(self):
        """
        Aborts rendering and shuts down
        """
        return
