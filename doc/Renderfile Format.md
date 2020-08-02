# Overview
The renderfile format specifies scenes to be rendered. These scenes are organized into render settings, a camera and multiple meshes and lights with their corresponding values and settings. The format is designed only for use in conjunction with the python blender renderer for PR Rendering.

# Format
``` json
[
    {
        "settings" :
        {
            "logLevel" : string,
            "storeBlend" : bool,
            "resolution" : [ int, int ],
            "pluginPath" : string,
            "shaderDirs" : [ string, ... ]
        },
        "camera" :
        {
            "position" : [ float, float, float ],
            "rotation" : [ float, float, float, float ],
            "scale" : [ float, float, float ],
            "fov" : [ float, float ],
            "aspect" : float,
            "shift" : [ float, float ],
            "nearZ" : float,
            "resultFile" : string,
            "dataOnly" : bool,
            "aaSamples" : int,
            "rayBounces" : int,
            "shadingOverride" : string,
            "shader" :
            {
                "name" : string,
                "textures" :
                [
                    {
                        "filePath" : string,
                        "colorSpace" : string,
                        "colorDepth" : string
                    },
                    ...
                ],
                "params" :
                {
                    "'shaderParam'" : 'value',
                    ...
                }
            }
        },
        "meshes" : [
            {
                "position" : [ float, float, float ],
                "rotation" : [ float, float, float, float ],
                "scale" : [ float, float, float ],
                "file" : string,
                "shader" :
                {
                    "name" : string,
                    "textures" :
                    [
                        {
                            "filePath" : string,
                            "colorSpace" : string,
                            "colorDepth" : string
                        },
                        ...
                    ],
                    "params" :
                    {
                        "'shaderParam'" : 'value',
                        ...
                    }
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
                        "'extraLightParam'" : 'value',
                        ...
                    }
            },
            ...
        ]
    },
    ...
]
```