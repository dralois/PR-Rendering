#pragma once

#pragma warning(push, 0)
#include <Shaders/Shading.h>
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
	// Add shader params
	//---------------------------------------
	void LabelShader::X_AddToJSON(PrettyWriter<stringstream>& writer) override
	{
		writer.Key("maskId");
		writer.Int(maskId);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------
	LabelShader(const string& name, const vector<OSLTexture>& textures, int maskId):
		OSLShader(name, textures),
		maskId(maskId)
	{
	}
};
