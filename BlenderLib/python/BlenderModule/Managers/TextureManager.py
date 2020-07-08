from ..Utils.TextureConverter import ConvertTexture
from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from ..Utils import FileName

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

class TextureFactory(object):

    def __init__(self, modulePath):
        self.__modulePath = modulePath
        self.__mapping = {}

    # Add image & convert accordingly to texture
    def AddTexture(self, imagePath, colorSpace, colorDepth):
        # Only add if image exists & has not been added
        if len(imagePath) > 0:
            if not imagePath in self.__mapping:
                self.__mapping[imagePath] = ConvertTexture(imagePath, colorSpace, colorDepth, self.__modulePath)

    # Get path of converted texture
    def GetTexture(self, imagePath):
        return self.__mapping.get(imagePath, "")
