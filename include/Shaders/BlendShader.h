#pragma once

#pragma warning(push, 0)
#include <Shaders/Shading.h>
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
	boost::filesystem::path maskImage;
	boost::filesystem::path blendImage;
	boost::filesystem::path objectsImage;
	bool forceScene;
	Eigen::Vector3f backColor;

	//---------------------------------------
	// Add shader params
	//---------------------------------------
	void BlendShader::X_AddToJSON(rapidjson::PrettyWriter<rapidjson::StringStream>& writer) override
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
	BlendShader(const std::string& name,
		const std::vector<OSLTexture>& textures,
		const boost::filesystem::path& maskImage,
		const boost::filesystem::path& blendImage,
		const boost::filesystem::path& objectsImage,
		bool forceScene, Eigen::Vector3f backColor) :
		OSLShader(name, textures),
		maskImage(maskImage),
		blendImage(blendImage),
		objectsImage(objectsImage),
		forceScene(forceScene),
		backColor(backColor)
	{
	}
};
