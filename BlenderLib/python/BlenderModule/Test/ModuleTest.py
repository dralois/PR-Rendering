from ..Managers.RenderManager import RenderManager

import json

# Runs test in main thread
def TestBase():
    # Load json file
    testScene = json.load(open(".\\BlenderModule\\Test\\module_test_base.json"))
    sceneStr = json.dumps(testScene)
    # Test rendering
    mgr = RenderManager()
    mgr.ProcessRenderfile(sceneStr)

# Runs renderfile update test in subprocess
def TestUpdate():
    # Load json files
    testScene = json.load(open(".\\BlenderModule\\Test\\module_test_base.json"))
    updateScene = json.load(open(".\\BlenderModule\\Test\\module_test_update.json"))
    testStr = json.dumps(testScene)
    updateStr = json.dumps(updateScene)
    # Test rendering & updating
    mgr = RenderManager()
    mgr.ProcessRenderfile(testStr)
    mgr.ProcessRenderfile(updateStr)
    mgr.UnloadProcesses()
    mgr.ProcessRenderfile(testStr)
