#pragma once

#pragma warning(push, 0)
#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Test metal shader for PBR rendering
//---------------------------------------
class MetalShader : public OSLShader
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
		return new MetalShader(*this);
	}

	//---------------------------------------
	// Constructors
	//---------------------------------------

	MetalShader() :
		OSLShader("metal_obj")
	{
	}
};
