#pragma once

#pragma warning(push, 0)
#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Shader for mask creation
//---------------------------------------
class LabelShader : OSLShader
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	int maskId;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void LabelShader::X_AddToJSON(JSONWriter writer) override
	{
		writer.Key("maskId");
		writer.Int(maskId);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	LabelShader(
		const std::string& name,
		const std::vector<Texture>& textures,
		int maskId
	):
		OSLShader(name, textures),
		maskId(maskId)
	{
	}
};
