import bpy

from ..Converters import Material

class MaterialObjects(object):

    def __init__(self):
        self.__materials : list[Material.MaterialConverter] = []

    def __del__(self):
        del self.__materials

    # Create and return new material
    def AddMaterial(self, name):
        newMat = Material.MaterialConverter(name)
        self.__materials.append(newMat)
        return newMat
