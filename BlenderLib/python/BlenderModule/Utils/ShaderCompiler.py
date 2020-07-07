from .Logger import GetLogger
from . import FileName, FileDir, FullPath

import os
import sys
import glob

logger = GetLogger()

# Compile and store every osl shader in a folder
def CompileFolder(shaderFolder, modulePath):
    logger.info(f"Compiling shaders in folder {shaderFolder}")
    # Glob all osl files, compile and store them
    for oslShader in glob.glob(shaderFolder + "\\*.osl"):
        bytecode = CompileFile(FullPath(oslShader), modulePath)
        StoreBytecode(bytecode, FileDir(oslShader) + FileName(oslShader) + ".oso")

# Compiles osl shader and returns bytecode
def CompileFile(sourceFile, modulePath):
    logger.info(f"Compiling shader {sourceFile}")
    # Store original path
    tempPath = os.environ["PATH"]
    pyDir = FullPath(modulePath + "\\appleseed\\lib")

    # Modify sys path and import tools
    sys.path.append(pyDir)
    sys.path.append(modulePath)
    import utils.path_util as path_utils

    # Modify env path and import appleseed
    binDir = path_utils.get_appleseed_bin_dir_path()
    os.environ['PATH'] += os.pathsep + binDir
    from appleseed._appleseedpython3 import ShaderCompiler

    # Undo modifications
    os.environ["PATH"] = tempPath
    sys.path.remove(modulePath)
    sys.path.remove(pyDir)

    # Create shader compiler
    oslStdPath = path_utils.get_stdosl_paths()
    compiler = ShaderCompiler(oslStdPath)

    # Open shader code
    codeFile = open(sourceFile, "r")
    sourceCode = codeFile.read()
    codeFile.close()

    # Compile and return code
    return compiler.compile_buffer(sourceCode)

# Stores compiled shader in file
def StoreBytecode(bytecode, codeFile):
    logger.info(f"Storing compiled shader {codeFile}")
    # Write bytecode out to file
    shaderFile = open(codeFile, "w")
    shaderFile.write(bytecode)
    shaderFile.close()
