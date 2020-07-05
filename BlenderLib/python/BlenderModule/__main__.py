from .Managers.RenderManager import RenderScenes, RenderSceneSingle

import json
import os

testScene = {
    "settings" :
    {
        "logLevel" : "info",
        "storeBlend" : True,
        "resolution" : [1280, 720],
        "plugin" : os.path.join(os.getcwd(), "blenderseed.zip"),
        "output" : os.path.join(os.getcwd(), "BlenderModule\\Test\\"),
        "shaderPaths" : [os.path.join(os.getcwd(), "BlenderModule\\Test\\")]
    },
    "camera" :
    {
        "position" : [6.0, -3.0, 5.0],
        "rotation" : [0.9, 0.0, 1.1],
        "result" : "test_render.png"
    },
    "meshes" : [
        {
            "position" : [-0.5, -0.5, 0.0],
            "scale" : [0.5, 0.5, 0.5],
            "file" : os.path.join(os.getcwd(), "BlenderModule\\Test\\module_test.obj"),
            "shader" : "module_test",
            "params" :
            {
                "color" : (0.5, 0.3, 0.7)
            }
        },
        {
            "position" : [1.0, 1.0, 0.0],
            "scale" : [0.5, 0.5, 0.5],
            "file" : os.path.join(os.getcwd(), "BlenderModule\\Test\\module_test.obj"),
            "shader" : "module_test",
            "params" :
            {
                "color" : (0.3, 0.7, 0.5)
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

temp = json.dumps(testScene)
testScene = json.loads(temp)

RenderSceneSingle(testScene)
