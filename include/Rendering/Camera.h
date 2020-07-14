#pragma once

#pragma warning(push, 0)
#include <Shaders/Shading.h>
#include <Renderfile.h>
#pragma warning(pop)

using namespace std;
using namespace Eigen;

//---------------------------------------
// Camera object wrapper for rendering
//---------------------------------------
class Camera : public RenderfileObject
{
private:
	//---------------------------------------
	// Fields
	//---------------------------------------

	Vector2f fieldOfView;
	Vector2f lensShift;
	string resultName;
	OSLShader* cameraEffect;

	//---------------------------------------
	// Methods
	//---------------------------------------

	virtual void X_AddToJSON(PrettyWriter<std::stringstream>& writer) override
	{
		writer.Key("fov");
		RenderfileData::AddEigenVector<Vector2f>(writer, fieldOfView);

		writer.Key("shift");
		RenderfileData::AddEigenVector<Vector2f>(writer, lensShift);

		writer.Key("result");
		RenderfileData::AddString(writer, resultName);

		// Camera does not always have an effect set
		if (cameraEffect)
		{
			writer.Key("shader");
			cameraEffect->AddToJSON(writer);
		}
	}

public:
	//---------------------------------------
	// Constructors
	//---------------------------------------

	Camera(Vector2f fieldOfView, Vector2f lensShift, const string& resultName, OSLShader* cameraEffect = NULL) :
		fieldOfView(fieldOfView),
		lensShift(lensShift),
		resultName(resultName),
		cameraEffect(cameraEffect)
	{
	}
};
