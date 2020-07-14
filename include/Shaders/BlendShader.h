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
	string maskImage;
	string blendImage;
	string objectsImage;
	bool forceScene;
	Eigen::Vector3f backColor;

	//---------------------------------------
	// Add shader params
	//---------------------------------------
	void BlendShader::X_AddToJSON(PrettyWriter<stringstream>& writer) override
	{
		writer.Key("maskImage");
		AddString(writer, maskImage);

		writer.Key("blendImage");
		AddString(writer, blendImage);

		writer.Key("objectsImage");
		AddString(writer, objectsImage);

		writer.Key("forceScene");
		writer.Bool(forceScene);

		writer.Key("backColor");
		RenderfileData::AddEigenVector<Vector3f>(writer, backColor);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------
	BlendShader(const string& name, const vector<OSLTexture>& textures,
		const string& maskImage, const string& blendImage, const string& objectsImage, bool forceScene, Eigen::Vector3f backColor) :
		OSLShader(name, textures),
		maskImage(maskImage),
		blendImage(blendImage),
		objectsImage(objectsImage),
		forceScene(forceScene),
		backColor(backColor)
	{
	}
};
