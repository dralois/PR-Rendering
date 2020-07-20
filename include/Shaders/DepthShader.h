#pragma once

#pragma warning(push, 0)
#include <Shaders/Shading.h>
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
	// Add shader params
	//---------------------------------------
	void DepthShader::X_AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) override
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
	DepthShader(const std::string& name, const std::vector<OSLTexture>& textures, float backgroundDepth, bool isBody) :
		OSLShader(name, textures),
		backgroundDepth(backgroundDepth),
		isBody(isBody)
	{
	}
};
