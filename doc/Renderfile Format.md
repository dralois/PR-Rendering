# Overview
The renderfile format specifies scenes to be rendered. These scenes are organized into render settings, a camera and multiple meshes and lights with their corresponding values and settings. The format is designed only for use in conjunction with the python blender renderer for PR Rendering.

# Format
``` json
[
    {
        "settings" :
        {
            "logLevel" : string,
            "resolution" : [ int, int ],
            "depthOnly" : bool,
            "output" : string,
            "storeBlend" : bool,
            "plugin" : string,
            "shaderPaths" : [ string, ... ],
            "texturePaths" : [ string, ... ]
        },
        "camera" :
        {
            "position" : [ float, float, float ],
            "rotation" : [ float, float, float, float ],
            "scale" : [ float, float, float ],
            "fov" : [ float, float ],
            "shift" : [ float, float ],
            "result" : string
        },
        "meshes" : [
            {
                "position" : [ float, float, float ],
                "rotation" : [ float, float, float, float ],
                "scale" : [ float, float, float ],
                "file" : string,
                "shader" : string,
                "params" :
                    {
                        "'param'" : 'value',
                        ...
                    }
            },
            ...
        ],
        "lights" : [
            {
                "position" : [ float, float, float ],
                "rotation" : [ float, float, float, float ],
                "scale" : [ float, float, float ],
                "type" : string,
                "color" : [ float, float, float ],
                "intensity" : float,
                "exposure" : float,
                "castsIndirect" : bool,
                "params" :
                    {
                        "'param'" : 'value',
                        ...
                    }
            },
            ...
        ]
    },
    ...
]
```