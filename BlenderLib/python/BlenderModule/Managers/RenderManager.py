from ..Utils.Importer import GetPaths, SetPaths
from .SceneManager import SceneProxy, Proxy2Scene

from typing import List
import multiprocessing

# Fetch functional and broken paths for Blender importing
__orgPaths, __bpyPaths = GetPaths()

# Renders a single scene
def RenderSceneSingle(sceneData : SceneProxy):
    # Create scene from proxy
    scene = Proxy2Scene(sceneData)
    # Render the scene
    scene.Render()

# Renders a collection of scenes in parallel
def RenderScenes(scenes : List[SceneProxy]):
    # Start render workers
    with multiprocessing.Pool(  maxtasksperchild=1,
                                initializer=SetPaths,
                                initargs=(__orgPaths, __bpyPaths)) as workers:
        workers.map(RenderSceneSingle, scenes)
        workers.close()
        workers.join()
