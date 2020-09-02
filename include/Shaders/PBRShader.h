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

	void X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("diffusePath");
		AddString(writer, diffuse->GetPath().string());

		writer.Key("metalness");
		AddFloat(writer, metalness);
	}

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual OSLShader* MakeCopy() const override
	{
		return new PBRShader(*this);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	PBRShader(
		Texture* diffuse,
		float metalness = 0.0f
	) :
		OSLShader("pbr_obj", {diffuse}),
		diffuse(diffuse),
		metalness(metalness)
	{
	}
};
