#pragma once

#pragma warning(push, 0)
#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Shader for PBR rendering
//---------------------------------------
class PBRShader : public OSLShader
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	Texture* diffuse;
	float metalness;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void PBRShader::X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("diffusePath");
		AddString(writer, diffuse->GetPath().string());

		writer.Key("metalness");
		AddFloat(writer, metalness);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	PBRShader(
		Texture* diffuse,
		float metalness = 0.0f
	) :
		OSLShader("pbr_obj", std::vector<Texture*>{diffuse}),
		diffuse(diffuse),
		metalness(metalness)
	{
	}
};
