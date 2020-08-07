#pragma once

#pragma warning(push, 0)
#include <Helpers/PathUtils.h>

#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Fullscreen shader for final blend
//---------------------------------------
class BlendShader : OSLShader
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	Texture* occlusion;
	Texture* original;
	Texture* rendered;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void BlendShader::X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("occlusionTexture");
		AddString(writer, occlusion->GetPath().string());

		writer.Key("originalImage");
		AddString(writer, original->GetPath().string());

		writer.Key("renderedImage");
		AddString(writer, rendered->GetPath().string());
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	BlendShader(
		Texture* occlusion,
		Texture* original,
		Texture* rendered
	) :
		OSLShader("blend_final", std::vector<Texture*>{occlusion, original, rendered}),
		occlusion(occlusion),
		original(original),
		rendered(rendered)
	{
	}
};
