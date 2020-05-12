#include <string.h>

#pragma warning(push, 0)
#include <ai.h>
#pragma warning(pop)

extern const AtNodeMethods* DepthShader;
extern const AtNodeMethods* LabelShader;
extern const AtNodeMethods* NullFilter;
extern const AtNodeMethods* BlendShader;

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
		{
			node->methods = (AtNodeMethods*)DepthShader;
			node->output_type = AI_TYPE_RGBA;
			node->name = "depthshader";
			node->node_type = AI_NODE_SHADER;
			strcpy(node->version, AI_VERSION);
			return true;
		}
		// Renders object IDs in red channel
		case LABEL_SHADER:
		{
			node->methods = (AtNodeMethods*)LabelShader;
			node->output_type = AI_TYPE_RGBA;
			node->name = "labelshader";
			node->node_type = AI_NODE_SHADER;
			strcpy(node->version, AI_VERSION);
			return true;
		}
		// Final color blend, renders objects on image
		case BLEND_SHADER:
		{
			node->methods = (AtNodeMethods*)BlendShader;
			node->output_type = AI_TYPE_RGB;
			node->name = "blendshader";
			node->node_type = AI_NODE_SHADER;
			strcpy(node->version, AI_VERSION);
			return true;
		}
		// Filter, calculates average red channel sum
		case NULL_FILTER:
		{
			node->methods = NullFilter;
			node->output_type = AI_TYPE_INT;
			node->name = "null_filter";
			node->node_type = AI_NODE_FILTER;
			strcpy(node->version, AI_VERSION);
			return true;
		}
		default:
			return false;
	}
}
