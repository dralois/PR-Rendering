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

	uchar maskRed;
	uchar maskGreen;
	uchar maskBlue;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void X_AddToJSON(JSONWriterRef writer) override
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
		cv::Vec3b color
	):
		OSLShader("label_obj"),
		maskRed(color[2]),
		maskGreen(color[1]),
		maskBlue(color[0])
	{
	}
};
