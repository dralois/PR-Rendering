#pragma once

#pragma warning(push, 0)
#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Shader for depth map generation
//---------------------------------------
class DepthShader : OSLShader
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	bool isBody;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void DepthShader::X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("isBody");
		writer.Bool(isBody);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	DepthShader(
		bool isBody
	) :
		OSLShader("depth_obj", (std::vector<Texture>)0),
		isBody(isBody)
	{
	}
};
