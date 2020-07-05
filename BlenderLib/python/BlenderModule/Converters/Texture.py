from ..Utils.Importer import ImportBpy
from ..Utils.Logger import GetLogger
from .Base import DataWrapper

# Blender for multiprocessing
bpy = ImportBpy()
logger = GetLogger()

# TODO
# Texture descriptor
class TextureData(DataWrapper):

    def __init__(self, blueprintID, cpy : DataWrapper = None):
        self.__isValid = False
        self.__texture : bpy.types.Image

        # TODO
        if cpy is None:
            # self.__texture = bpy.data.images.new(blueprintID)
        elif isinstance(cpy, TextureData):
            self.__texture = cpy.Blueprint.copy()
            self.__texture.name = blueprintID
            self.__isValid = True
        else:
            raise TypeError

        super().__init__(self.__texture)

    # TODO
    # Override: Cleanup & remove texture
    def _Cleanup(self):
        pass

    # Override: Get if texture valid
    @property
    def Valid(self):
        return self.__isValid

    # TODO
    # Override: Create from json data
    def CreateFromJSON(self, data : dict):
        pass