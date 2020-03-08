#ifndef ai_common_mtds
#include <ai.h>
#endif
#include <stdio.h>

extern AtNodeMethods *DepthMethods;
extern AtNodeMethods *LabelMethods;
extern AtNodeMethods *CustomNullFilterMtd;
extern AtNodeMethods *BlendMethods;

enum SHADERS
{
    DEPTH_SHADER,
    LABEL_SHADER,
    BLEND_SHADER,
    NULL_FILTER
};

node_loader
{
    switch (i)
    {
    // Renders depth (distance) in red channel
    case DEPTH_SHADER:
        node->methods = (AtNodeMethods *)DepthMethods;
        node->output_type = AI_TYPE_RGBA;
        node->name = "depthshader";
        node->node_type = AI_NODE_SHADER;
        break;
    // Renders object IDs in red channel
    case LABEL_SHADER:
        node->methods = (AtNodeMethods *)LabelMethods;
        node->output_type = AI_TYPE_RGBA;
        node->name = "labelshader";
        node->node_type = AI_NODE_SHADER;
        break;
    // Final color blend, renders objects on image
    case BLEND_SHADER:
        node->methods = (AtNodeMethods *)BlendMethods;
        node->output_type = AI_TYPE_RGB;
        node->name = "blendshader";
        node->node_type = AI_NODE_SHADER;
        break;
    // Filter, calculates average red channel sum
    case NULL_FILTER:
        node->methods = CustomNullFilterMtd;
        node->output_type = AI_TYPE_INT;
        node->name = "null_filter";
        node->node_type = AI_NODE_FILTER;
        break;
    default:
        return false;
    }

    sprintf(node->version, AI_VERSION);
    return true;
}