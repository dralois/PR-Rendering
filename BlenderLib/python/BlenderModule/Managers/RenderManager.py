from typing import List

import multiprocessing

from ..Utils.Importer import ImportBPY

org_sys, bpy_sys = ImportBPY()
print(org_sys)
print(bpy_sys)

from .SceneManager import SceneProxy, Proxy2Scene

def RenderSceneSingle(scene : SceneProxy):
    # Create scene from proxy
    scene = Proxy2Scene(self.__scene)
    # Load blenderseed & setup
    scene.Setup()
    # Finally render the scene
    scene.Render()
    pass

def RenderScenes(scenes : List[SceneProxy]):
    with multiprocessing.Pool(maxtasksperchild=1) as workers:
        workers.map(RenderSceneSingle, scenes)
        workers.close()
        workers.join()
