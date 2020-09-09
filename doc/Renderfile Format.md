# Overview
The renderfile format specifies a scene to be rendered. These scenes are organized into render settings, cameras to render and multiple meshes and lights with their corresponding values and settings. The format is designed only for use in conjunction with the python blender renderer for PR Rendering.

# Format
``` json
{
    "settings" :
    {
        "logLevel" : string,
        "storeBlend" : bool,
        "pluginPath" : string,
        "shaderDirs" : [ string, ... ]
    },
    "cameras" : [
        {
            "position" : [ float, float, float ],
            "rotation" : [ float, float, float, float ],
            "scale" : [ float, float, float ],
            "fov" : [ float, float, float ],
            "shift" : [ float, float ],
            "resultFile" : string,
            "resolution" : [ int, int ],
            "dataOnly" : bool,
            "aaSamples" : int,
            "rayBounces" : int,
            "shadingOverride" : string
        },
        ...
    ],
    "meshes" : [
        {
            "position" : [ float, float, float ],
            "rotation" : [ float, float, float, float ],
            "scale" : [ float, float, float ],
            "objectID" : int,
            "file" : string,
            "indirect" : bool,
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
}
```

# Description

## Settings
Contains universal settings & paths, is not updated when processing the same renderfile again

## Cameras
Array of cameras that should be rendered. Each camera has extrinsics & intrinsics as well as render settings. If the same renderfile is processed again, existing cameras will not be updated but rather destroyed and created again.

## Meshes
Array of meshes that should be used for rendering. Each mesh has a transform, unique ID and path to the actual file. Currently Wavefront and glTF meshes are supported. Mesh may be marked as indirect, which disables direct camera & shadow rays. Each mesh also has a shader that defines its material. If the same renderfile is processed again, existing meshes are updated and only reload from disk if the path changed.

## Shaders
Shaders are identified by name, a list of available shaders with names can be found in the shader class. Each shader has an array of textures it uses. Each shader also specifies an arbitrary amount of input parameters.

## Textures
Textures are identified by file path and transformed into a render-friendly format when first loaded. They may also specify a color space and depth (precision).

## Lights
Array of lights that should be used for rendering. Each light has a transform and general parameters as well as optionally light specific parameters. If the same renderfile is processed again, existing lights will not be updated but rather destroyed and created again.
