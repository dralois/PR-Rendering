#pragma once

#pragma warning(push, 0)
#include <Rendering/Shader.h>
#include <Rendering/Texture.h>
#pragma warning(pop)

//---------------------------------------
// Shader for depth map generation
//---------------------------------------
class DepthShader : public OSLShader
{
protected:
	//---------------------------------------
	// Fields
	//---------------------------------------

	float camNear;
	float camFar;

	//---------------------------------------
	// Methods
	//---------------------------------------

	void DepthShader::X_AddToJSON(JSONWriterRef writer) override
	{
		writer.Key("clipNear");
		AddFloat(writer, camNear);
		writer.Key("clipFar");
		AddFloat(writer, camFar);
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	DepthShader(
		float camNear,
		float camFar
	) :
		OSLShader("depth_obj"),
		camNear(camNear),
		camFar(camFar)
	{
	}
};
