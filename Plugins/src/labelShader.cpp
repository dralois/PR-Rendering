#pragma once

#ifndef ai_common_mtds
#include <ai.h>
#endif
#include <string.h>
#include <iostream>
#include <math.h>

AI_SHADER_NODE_EXPORT_METHODS(LabelShader);

enum LabelParams
{
	p_id
};

node_parameters
{
	AiParameterInt("id", 0);
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
	// Renders object ID into red channel
	AtRGBA rgba;
	rgba.r = AiShaderEvalParamInt(p_id);
	sg->out.RGBA = rgba;
}
