import ComputeShader as CS
import numpy as np

from numpy.typing import ArrayLike

import os

def GetGroupSizes():
    info = CS.Info()
    return (info["WORK_GROUP_COUNT"],
            info["WORK_GROUP_SIZE"],
            info["WORK_GROUP_INVOCATIONS"])

class ShaderManager(object):

    def __init__(self, path):
        self.path = path
        self.shader = None
        self.SSBOs = {}

    def __enter__(self):
        # Compute shader must exist
        if os.path.exists(self.path):
            with open(self.path) as stream:
                # Read file & create shader
                source = stream.read(-1)
                self.shader = CS.NewCS(source)
        # Start timer
        CS.StartTimer()
        # Create was successful
        return self

    def __exit__(self, *args):
        # Shader must have been created
        if self.shader is not None:
            # Delete all SSBOs
            for ssbo, _, _, _ in self.SSBOs.values():
                CS.DeleteSSBO(ssbo)
            # Then delete shader
            CS.DeleteCS(self.shader)
        # Stop and return timer
        print(f"{self.path} completed in {CS.GetTimer()}")

    def SetData(self, binding, data : ArrayLike):
        # Shader must have been created
        if self.shader is not None:
            # Convert np array to bytes
            packed = data.tobytes()
            # Either update or create buffer
            if binding in self.SSBOs:
                ssbo = self.SSBOs[binding][0]
                CS.UpdateSSBO(ssbo, packed)
            else:
                ssbo = CS.NewSSBO(packed)
                CS.UseSSBO(ssbo, self.shader, binding)
            # Store info
            self.SSBOs[binding] = (ssbo, data.shape, data.dtype, data.nbytes)

    def GetData(self, binding):
        # Shader must have been created & buffer must exist
        if self.shader is not None:
            if binding in self.SSBOs:
                # Read values based on info
                ssbo, shape, format, size = self.SSBOs[binding]
                return np.ndarray(shape, dtype=format, buffer=CS.ReadSSBO(ssbo, size))

    def StartCompute(self, worksize):
        # Shader must have been created
        if self.shader is not None:
            CS.UseCS(self.shader, worksize)
