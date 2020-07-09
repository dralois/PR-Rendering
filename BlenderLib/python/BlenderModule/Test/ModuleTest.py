from ..Managers.RenderManager import RenderScenes, RenderSceneSingle

import json

testScene = {
    "settings" :
    {
        "logLevel" : "info",
        "storeBlend" : False,
        "resolution" : [1280, 720],
        "pluginDir" : "blenderseed.zip",
        "outputDir" : "Test",
        "shaderDirs" : ["Test"]
    },
    "camera" :
    {
        "position" : [6.0, -3.0, 5.0],
        "rotation" : [0.9, 0.0, 1.1],
        "result" : "test_render",
    },
    "meshes" : [
        {
            "position" : [-0.5, -0.5, 0.0],
            "scale" : [0.5, 0.5, 0.5],
            "file" : "Test\\module_test.obj",
            "shader" :
            {
                "name" : "simple_texture",
                "textures" : [
                    {
                        "filePath" : "Test\\module_test.png"
                    }
                ],
                "params" :
                {
                    "filename" : "Test\\module_test.png"
                }
            }
        },
        {
            "position" : [1.0, 1.0, 0.0],
            "scale" : [0.5, 0.5, 0.5],
            "file" : "Test\\module_test.obj",
            "shader" :
            {
                "name" : "module_test",
                "params" :
                {
                    "color" : (0.3, 0.7, 0.5)
                }
            }
        }
    ],
    "lights" : [
        {
            "position" : [3.0, -4.2, 5.0],
            "type" : "POINT",
            "intensity" : 100
        }
    ]
}

# Runs test in main thread
def TestMain():
    global testScene
    # Tests serialization
    temp = json.dumps(testScene)
    testScene = json.loads(temp)
    # Tests rendering
    RenderSceneSingle(testScene)

# Runs test in subprocess
def TestSubprocess():
    global testScene
    # Tests serialization
    sceneStr = json.dumps([testScene])
    # Tests rendering
    RenderScenes(sceneStr)
