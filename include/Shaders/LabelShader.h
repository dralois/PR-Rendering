#pragma once

#pragma warning(push, 0)
#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Shader for mask creation
//---------------------------------------
class LabelShader : public OSLShader
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	int maskRed;
	int maskGreen;
	int maskBlue;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void LabelShader::X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("maskRed");
		writer.Int(maskRed);
		writer.Key("maskGreen");
		writer.Int(maskGreen);
		writer.Key("maskBlue");
		writer.Int(maskBlue);
	}

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual OSLShader* MakeCopy() const override
	{
		return new LabelShader(*this);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	LabelShader(
		int red,
		int green,
		int blue
	):
		OSLShader("label_obj"),
		maskRed(red),
		maskGreen(green),
		maskBlue(blue)
	{
	}
};
