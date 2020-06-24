from ..Utils.Importer import DoImport
from .Base import DataWrapper

# Blender for multiprocessing
bpy = DoImport()

import os

# Shader descriptor
class ShaderData(DataWrapper):

    def __init__(self, name, cpy = None):
        self.__isValid = False
        self.__shaderCode : bpy.types.Text

        if cpy is None:
            self.__shaderCode = bpy.data.texts.new("osl_" + name)
        elif isinstance(cpy, ShaderData):
            self.__shaderCode = cpy.Blueprint.copy()
        else:
            raise TypeError

        super().__init__(name, self.__shaderCode)

    # Override: Cleanup & remove text block
    def _Cleanup(self):
        if self.__shaderCode is not None:
            bpy.data.texts.remove(self.__shaderCode)

    # Override: Get if shader valid
    @property
    def Valid(self):
        return self.__isValid

    # Get shader code block
    @property
    def ShaderCode(self):
        return self.__shaderCode

    # Create shader text block from an OSL shader file
    def CreateFromFile(self, filePath):
        file = os.path.abspath(filePath)
        # Only load if normalized path exists & not yet created
        if len(filePath) > 0 and not self.__isValid:
            # Create new
            blueprintName = self.BlueprintID
            newShader = bpy.data.texts.load(file)
            newShader.name = blueprintName
            # Handle internals
            self._Update(newShader)
            self.__shaderCode = newShader
            self.__isValid = True
