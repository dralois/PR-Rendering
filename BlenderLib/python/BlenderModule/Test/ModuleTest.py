from threading import Thread
from ..Managers.RenderManager import RenderManager

import json

# Runs test in main thread
def TestBase():
    # Load json file
    testScene = json.load(open("./BlenderModule/Test/module_test_base.json"))
    sceneStr = json.dumps(testScene)
    # Test rendering
    mgr = RenderManager(1)
    mgr.ProcessRenderfile(sceneStr, 60, 0)

# Runs renderfile update test in subprocess
def TestUpdate():
    # Load json files
    testScene = json.load(open("./BlenderModule/Test/module_test_base.json"))
    updateScene = json.load(open("./BlenderModule/Test/module_test_update.json"))
    testStr = json.dumps(testScene)
    updateStr = json.dumps(updateScene)
    # Test rendering & updating
    mgr = RenderManager(1)
    mgr.ProcessRenderfile(testStr, 60, 0)
    mgr.ProcessRenderfile(updateStr, 60, 0)
    mgr.UnloadProcess(0)
    mgr.ProcessRenderfile(testStr, 60, 0)

# Runs test multithreaded
def TestMultithread():
    # Load json file
    testScene = json.load(open("./BlenderModule/Test/module_test_base.json"))
    scene1Str = json.dumps(testScene)
    scene2Str = json.dumps(testScene)
    mgr = RenderManager(2)
    # Create threads
    thr1 = Thread(target=mgr.ProcessRenderfile, args=(scene1Str, 20, 0))
    thr2 = Thread(target=mgr.ProcessRenderfile, args=(scene2Str, 60, 1))
    # Test rendering
    thr1.start()
    thr2.start()
    thr1.join()
    thr2.join()
