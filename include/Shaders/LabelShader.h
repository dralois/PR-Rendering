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

	void LabelShader::X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("maskId");
		writer.Int(maskId);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	LabelShader(
		int maskId
	):
		OSLShader("label_obj", (std::vector<Texture>)0),
		maskId(maskId)
	{
	}
};
