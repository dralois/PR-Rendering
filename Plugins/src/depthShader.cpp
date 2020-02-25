#ifndef ai_common_mtds
#include <ai.h>
#endif
#include <string.h>
#include <iostream>
#include <math.h>

AI_SHADER_NODE_EXPORT_METHODS(DepthMethods);

enum DepthParams
{
    p_force_val,
    p_is_body
};

node_parameters
{
    AiParameterFlt("force_val", 0);
    AiParameterBool("is_body", false);
}

node_initialize
{
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
    // Get camera and fragment position
    AtVector cam = sg->Ro;
    AtVector vert = sg->P;

    // Background value
    float x = AiShaderEvalParamFlt(p_force_val);

    // If it is an object use distance to camera instead
    if (AiShaderEvalParamBool(p_is_body))
        x = sqrt(pow(sg->Ro.x - sg->P.x, 2) + pow(sg->Ro.y - sg->P.y, 2) + pow(sg->Ro.z - sg->P.z, 2))*10;

    // ?
    if(AiShaderEvalParamFlt(p_force_val)>1){
    } 

    // ?
    sg->out.FLT = 20.f;
    sg->out.INT = 30;
    sg->out.UINT = 40;
 
    // Save distance in red channel
    AtRGBA rgba;
    rgba.r = x;
    rgba.g = 1;
    rgba.b = 1;

    // Return distance
    sg->out.RGBA = rgba;
}
