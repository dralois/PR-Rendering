from .Logger import get_logger

import os
import sys
import glob

logger = get_logger()

# Compile and store every osl shader in a folder
def CompileFolder(shaderFolder, modulePath):
    logger.info(f"Compiling shaders in folder {shaderFolder}")
    # Glob all osl files, compile and store them
    for oslShader in glob.glob(shaderFolder + "*.osl"):
        bytecode = CompileFile(os.path.abspath(oslShader), modulePath)
        StoreBytecode(bytecode, os.path.splitext(oslShader)[0] + ".oso")

# Compiles osl shader and returns bytecode
def CompileFile(sourceFile, modulePath):
    logger.info(f"Compiling shader {sourceFile}")
    # Modify path and import tools
    sys.path.append(modulePath)
    import utils.path_util as path_utils
    path_utils.load_appleseed_python_paths()
    from appleseed._appleseedpython3 import ShaderCompiler
    sys.path.remove(modulePath)

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
