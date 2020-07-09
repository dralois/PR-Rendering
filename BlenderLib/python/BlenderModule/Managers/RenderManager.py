from ..Utils.Importer import GetPaths, SetPaths
from ..Utils import FullPath, FileDir
from .SceneManager import CreateFromJSON, Scene

from typing import List
from os import chdir
import multiprocessing
import json

# Fetch functional and broken paths for Blender importing
__orgPaths, __bpyPaths = GetPaths()

# Renders a single scene
def RenderSceneSingle(sceneData):
    # Change working dir in process
    chdir(FullPath(f"{FileDir(__file__)}\\..\\"))
    # Create scene from proxy
    scene = CreateFromJSON(sceneData)
    # Render the scene
    scene.Render()

# Renders a collection of scenes in parallel
def RenderScenes(renderfileString):
    # Parse renderfile
    scenes = json.loads(renderfileString)
    # Start render workers
    with multiprocessing.Pool(  maxtasksperchild=1,
                                initializer=SetPaths,
                                initargs=(__orgPaths, __bpyPaths)) as workers:
        workers.map(RenderSceneSingle, scenes)
        workers.close()
        workers.join()
