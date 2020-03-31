#include <ai.h>
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
	AtRGBA rgba(0,0,0,1);
	rgba.r = AiShaderEvalParamInt(p_id);
	sg->out.RGBA() = rgba;
}
