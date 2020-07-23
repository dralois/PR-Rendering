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

	ModifiablePath maskImage;
	ModifiablePath blendImage;
	ModifiablePath objectsImage;
	bool forceScene;
	Eigen::Vector3f backColor;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void BlendShader::X_AddToJSON(JSONWriter writer) override
	{
		writer.Key("maskImage");
		AddString(writer, maskImage.string());

		writer.Key("blendImage");
		AddString(writer, blendImage.string());

		writer.Key("objectsImage");
		AddString(writer, objectsImage.string());

		writer.Key("forceScene");
		writer.Bool(forceScene);

		writer.Key("backColor");
		AddEigenVector<Eigen::Vector3f>(writer, backColor);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	BlendShader(
		const std::string& name,
		const std::vector<Texture>& textures,
		ReferencePath maskImage,
		ReferencePath blendImage,
		ReferencePath objectsImage,
		bool forceScene,
		Eigen::Vector3f backColor
	) :
		OSLShader(name, textures),
		maskImage(maskImage),
		blendImage(blendImage),
		objectsImage(objectsImage),
		forceScene(forceScene),
		backColor(backColor)
	{
	}
};
