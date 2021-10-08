#pragma once

#pragma warning(push, 0)
#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Test glass shader for PBR rendering
//---------------------------------------
class GlassShader : public OSLShader
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	//---------------------------------------
	// Methods
	//---------------------------------------

	void X_AddToJSON(JSONWriterRef writer) const override { }

public:
	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual OSLShader* MakeCopy() const override
	{
		return new GlassShader(*this);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	GlassShader() :
		OSLShader("glass_obj")
	{
	}
};
