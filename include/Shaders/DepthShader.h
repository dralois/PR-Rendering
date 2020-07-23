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

	float backgroundDepth;
	bool isBody;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void DepthShader::X_AddToJSON(JSONWriter writer) override
	{
		writer.Key("backgroundDepth");
		AddFloat(writer, backgroundDepth);

		writer.Key("isBody");
		writer.Bool(isBody);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	DepthShader(
		const std::string& name,
		const std::vector<Texture>& textures,
		float backgroundDepth,
		bool isBody
	) :
		OSLShader(name, textures),
		backgroundDepth(backgroundDepth),
		isBody(isBody)
	{
	}
};
